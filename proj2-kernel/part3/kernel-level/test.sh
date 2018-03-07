#!/bin/bash
make
sudo rmmod elevator
sudo insmod elevator.ko
../user-level/consumer.x --start
for run in {1..15}
do
    ../user-level/producer.x
    ../user-level/producer.x
    ../user-level/producer.x
    sleep 1
done
while true; do
    cat /proc/elevator
    printf "\n"
    sleep 2
done
