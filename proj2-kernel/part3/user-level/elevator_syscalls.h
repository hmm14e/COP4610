#ifndef __ELEVATOR_SYSCALLS_H
#define __ELEVATOR_SYSCALLS_H

/**
 * start_elevator - initalizes an empty elevator at floor 1 and IDLE status
 * @return:  0 if successful, 1 if already active, -ERRORNUM if error
 */
int start_elevator(void);


/**
 * issue_request - creates a passenger to be added to a floor queue
 * @return: 1 if request is not valid (out of range), 0 otherwise
 */
int issue_request(int passenger_type, int start_floor, int dest_floor);

/**
 * stop_elevator - elevator stops accepting requests and unloads all current passenger
 * @return: 1 if elevator already deactivating, 0 otherwise
 */
int stop_elevator(void);

#endif /* __ELEVATOR_SYSCALLS_H */
