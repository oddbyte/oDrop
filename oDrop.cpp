#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <limits.h>
#include <string.h>
#include <linux/securebits.h>
#include <sys/capability.h>
#include <fcntl.h>
#include <sys/stat.h>

void print_help(const char *prog_name) {
    fprintf(stderr, "Usage: %s <command> [args...]\n", prog_name);
    fprintf(stderr, "Executes a command with restricted capabilities.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  <command>   The command to execute.\n");
    fprintf(stderr, "  [args...]   Optional arguments to pass to the command.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s ls -l\n", prog_name);
    fprintf(stderr, "  %s /usr/bin/grep 'text' file.txt\n", prog_name);
}

char *find_command_in_path(const char *cmd) {
    char *path_env = getenv("PATH");
    char *path = strdup(path_env);
    char *token = strtok(path, ":");

    while (token != NULL) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", token, cmd);
        if (access(full_path, X_OK) == 0) {
            free(path);
            return strdup(full_path);
        }
        token = strtok(NULL, ":");
    }

    free(path);
    return NULL;
}

int check_cap_setpcap(const char *path) {
    struct __user_cap_header_struct cap_header;
    struct __user_cap_data_struct cap_data[2];

    // Set up capability header for the binary
    cap_header.version = _LINUX_CAPABILITY_VERSION_3;
    cap_header.pid = 0; // 0 means we want to check the capabilities of the binary itself

    // Open the binary file to check its capabilities
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening binary");
        return -1; // Return -1 if there is an error opening the binary
    }

    // Use capget to retrieve the capabilities for the binary
    if (capget(&cap_header, cap_data) == -1) {
        perror("Error getting capabilities");
        close(fd);
        return -1;
    }

    close(fd); // Always close the file descriptor

    // Check if CAP_SETPCAP is in the permitted capabilities
    if (cap_data[0].permitted & (1 << CAP_SETPCAP)) { // Ensure proper bit mask
        return 1; // CAP_SETPCAP is permitted
    }

    return 0; // CAP_SETPCAP is not permitted
}

int main(int argc, char *argv[]) {
    if (check_cap_setpcap(argv[0]) != 1) {
        fprintf(stderr, "Error: CAP_SETPCAP is not set on the binary. Use 'sudo setcap cap_setpcap=eip %s' to set it.\n", argv[0]);
        return 1;
    }

    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    char command_path[PATH_MAX];

    if (realpath(argv[1], command_path) == NULL) {
        char *found_path = find_command_in_path(argv[1]);
        if (found_path != NULL) {
            strncpy(command_path, found_path, sizeof(command_path));
            free(found_path);
        } else {
            fprintf(stderr, "Error: Command not found: %s\n", argv[1]);
            print_help(argv[0]);
            return 1;
        }
    } else {
        strncpy(command_path, argv[1], sizeof(command_path));
    }


    if (prctl(PR_SET_SECUREBITS,
              SECBIT_KEEP_CAPS_LOCKED |
              SECBIT_NO_SETUID_FIXUP |
              SECBIT_NO_SETUID_FIXUP_LOCKED |
              SECBIT_NOROOT |
              SECBIT_NOROOT_LOCKED |
              SECBIT_NO_CAP_AMBIENT_RAISE) == -1) {
        perror("Error setting secure bits");
        return 1;
    }

    struct __user_cap_header_struct cap_header;
    struct __user_cap_data_struct cap_data[2];

    cap_header.version = _LINUX_CAPABILITY_VERSION_3;
    cap_header.pid = 0;

    if (capget(&cap_header, cap_data) == -1) {
        perror("Error getting capabilities");
        return 1;
    }

    cap_data[0].effective &= ~CAP_SETPCAP;
    cap_data[0].permitted &= ~CAP_SETPCAP;

    if (capset(&cap_header, cap_data) == -1) {
        perror("Error setting capabilities");
        return 1;
    }

    char *exec_args[argc];
    exec_args[0] = command_path;

    for (int i = 2; i < argc; i++) {
        exec_args[i - 1] = argv[i];
    }
    exec_args[argc - 1] = NULL;

    execvp(exec_args[0], exec_args);

    perror("Error executing command");
    return 1;
}

