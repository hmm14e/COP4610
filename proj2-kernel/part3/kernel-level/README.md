## Implementation details

### Main parts
* `Passenger` implementation
    * Implements a simple struct to hold the *passenger_type* and *destination_floor*.
    * Load information (units and weight) are located in a global lookup table.
* `Elevator` implementation
    * Implement the logic to move between floors, load and unload passengers from/to a floor. Scheduling is deferred to the `Controller`.
    * This can be considered the *consumer*, as it removes passengers from the floor queue.
* `Floor` implementation
    * Implement the logical representation of a single floor, which is in essence a FIFO queue, its corresponding lock, and some metric variables.
* `Controller` implementation
    * *not sure about this yet*
    * Implement the logic to handle `issue_request` syscalls and to schedule the elevator.
    * The `Controller` decides how to schedule by using the current floor state and elevator state (current_floor, passengers, etc).
    * This can be considered the *producer*, as it appends the requests to their corresponding floor queue.
* ProcFS functions
    * Implement the handlers to writing status to the /proc/elevator file.
    * When /proc/elevator is read, the elevator and floor status are written to the file.
* Module functions
    * Implement the initialization and teardown logic of the module.
    * On initialization, the `Elevator` ,  `Floor` array, and `Controller` variables are initialized.

### Design Decisions
* The `Elevator` and `Controller` class instances are scoped as global variables, as the controller needs to know about the elevator to schedule and the controller needs to schedule the elevator via "class functions".
* Floors are represented as a global array of `Floor` structs, as both the `Controller` and `Elevator` needs to access and modify them. It is assured that operations are thread-safe, as the `Floor` implementation uses an internal lock for any queue operations.
* Scheduling is deferred to the `Controller` because in the case that there were multiple elevators, there would need to be a global scheduler, rather than intra-elevator schedulers.
