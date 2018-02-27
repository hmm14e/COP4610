#!/bin/bash
set -x
mkdir /usr/src/test_kernel/elevator
sudo cp Makefile *.c /usr/src/test_kernel/elevator 
sudo cp files/Makefile /usr/src/test_kernel/Makefile
sudo cp files/syscall_64.tbl /usr/src/test_kernel/arch/x86/entry/syscalls/syscall_64.tbl 
