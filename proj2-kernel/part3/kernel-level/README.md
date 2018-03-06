## Implementation details

### Main parts
* `Passenger` implementation
    * Implements a simple struct to hold the *passenger_type* and *destination_floor*.
    * Load information (units and weight) are located in a global lookup table.
* `Elevator` implementation
    * Implements the logic to move between floors, load and unload passengers from/to a floor.
    * Implements `elevator_run` -- threaded function in charge of the scheduling (currently the [SCAN algortihm](https://en.wikipedia.org/wiki/Elevator_algorithm))
* `Floor` implementation
    * Implements the logical representation of a single floor, which is in essence a FIFO queue, its corresponding lock, and some metric variables.
* ProcFS functions
    * Implements the handlers to writing status to the /proc/elevator file.
    * When /proc/elevator is read, the elevator and floor status are written to the file.
* Module functions
    * Implements the initialization and teardown logic of the module.
    * On initialization, the `Elevator` ,  `Floor` array, and `Controller` variables are initialized.

