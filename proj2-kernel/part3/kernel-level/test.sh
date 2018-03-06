#!/bin/bash
set -x
N_TIMES=$1
make
sudo rmmod elevator
sudo insmod elevator.ko
../user-level/consumer.x --start
for run in {1..$N_TIMES}
do
    ../user-level/producer.x
    sleep .5
done
while true; do
    cat /proc/elevator
    ../user-level/producer.x
    ../user-level/producer.x
    ../user-level/producer.x
    sleep 2
done
