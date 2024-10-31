# oDrop
## Drop the capabilities you dont need

This is an application that drops as many [capabilities](https://man7.org/linux/man-pages/man7/capabilities.7.html) as possible. Can be used to make an app running as root less powerful, or to prevent an app from gaining root using a Set-UID binary (like sudo).

```
Usage: ./odrop <command> [args...]
Executes a command with restricted capabilities.

Options:
  <command>   The command to execute.
  [args...]   Optional arguments to pass to the command.

Examples:
  ./odrop ls -l
  ./odrop /usr/bin/grep 'text' file.txt
```

## Example:
### Can be used to block `sudo` from running
```
oddbyte@oddbyte:/$ ./odrop bash
oddbyte@oddbyte:/$ sudo su
sudo: unable to change to root gid: Operation not permitted
sudo: error initializing audit plugin sudoers_audit
oddbyte@oddbyte:/$ exit
exit
oddbyte@oddbyte:/$ sudo su
root@oddbyte:/#
```

## Warning:
> If you use this to make root less powerful, please remember that root is the owner of most system files and directories. This means that even though the root shell is less powerful (it cannot bypass file permissions checks anymore, so it cannot edit your /home folder), it can still cause huge damage to the system, and probably can break out of the restricted environment via making a rouge systemd service.
> This should only be used for limiting an already limited user.
