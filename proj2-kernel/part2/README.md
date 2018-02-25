## Part 2 - Kernel Module
In Unix-like operating systems, time is sometimes specified to be the seconds since the Unix Epoch (January 1st, 1970). You will create a kernel module called *my_xtime* that calls `current_kernel_time()` and stores the value. `current_kernel_time()` holds the number of seconds and nanoseconds since the Epoch.

When *my_xtime* is loaded (using insmod), it should create a proc entry called /proc/timed. When *my_xtime* is unloaded (using rmmod), /proc/timed should be removed.

On each read you will use the proc interface to both print the current time as well as the amount of time that's passed since the last call (if valid).

Example usage:

```
$ cat /proc/timed
current time: 1518647111.760933999
$ sleep 1
$ cat /proc/timed
current time: 1518647112.768429998
elapsed time: 1.007495999

$ sleep 3
$ cat /proc/timed
current time: 1518647115.774925999
elapsed time: 3.006496001
$ sleep 5
$ cat /proc/timed
current time: 1518647120.780421999
elapsed time: 5.005496000
```
