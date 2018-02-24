## Part 1 - System-call Tracing

Write a program that uses exactly eight system calls. You will not receive points if the program contains more or fewer than eight. The system calls available to your machine can be found within /usr/include/unistd.h. Further, you can use the command line tool, strace, to intercept and record the system calls called by a process.

Once you've written this program, execute the following commands:

```
$ gcc -o part1.x part1.c
$ strace -o log part1.x
```

Look at the log file to figure out how many system calls your program is calling. To reduce the length of the output from strace, try to minimize the use of other function calls (e.g. stdlib.h) in your program.
