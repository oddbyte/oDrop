# oDrop
## Drop the capabilities you dont need

Helps put a roadblock in front of people who try to abuse SUID binaries such as `su`. Don't let your webserver get root!

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
