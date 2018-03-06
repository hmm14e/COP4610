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
### Scheduling
* When the elevator doesn't have anyone in it, it's status is set to *IDLE*
* In the `elevator_run` loop, there are two cases:
   * The elevator is IDLE, which in this case the elevator will 
      * Do a search through the floors looking for someone to service, if someone is found we move to that a floor, pick them up, and start a scan in their destination direction
      * If no one is found, just call `schedule` (voluntarily give control to scheduler) and wait
   * The elevator is UP or DOWN, which in this case the elevator will
      * Unload all passengers who are at their destination floor.
         * If we have unloaded everybody, then we are done with the current scan, and can start a new one
         * Else we continue with the current scan
 * This scheduling is very naive, for a few reasons
   * It doesn't take advantage of the fact that most on people on floor 2..N are going to floor 1
   * It doesn't take into account clusters of requests
   * When starting a new scan, it starts looking for people starting at floor 1 .. N, rather than close to where it already is
   
      
   
### TODO
* Handle fractional load units
* Handle `mutex_lock_interruptible` signals (i.e. when it doesn't return 0)
* Handle `floor_create` allocation failure
* Test thoroughly
