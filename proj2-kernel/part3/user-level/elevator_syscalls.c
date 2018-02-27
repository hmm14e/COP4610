#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#include "elevator_syscalls.h"

#define __NR_START_ELEVATOR 333
#define __NR_ISSUE_REQUEST 334
#define __NR_STOP_ELEVATOR 335


int start_elevator()
{
    return syscall(__NR_START_ELEVATOR);
}


int issue_request(int passenger_type, int start_floor, int destination_floor)
{
    return syscall(__NR_ISSUE_REQUEST);
}


int stop_elevator()
{
    return syscall(__NR_STOP_ELEVATOR);
}
