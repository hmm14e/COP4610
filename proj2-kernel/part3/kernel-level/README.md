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
    * On initialization, the `Elevator` and `Floor` array variables are initialized.
### Scheduling
* When the elevator doesn't have anyone in it, its status is set to *IDLE*
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
### Notes
* There is no floating point arithmetic allowed in kernel-mode, so handling fractional weight units (child 0.5) is tricky. Our implementation uses an internal variable on `Floor` and `Elevator`, `load_in_weight_half`, to keep track of whether there is an even or odd number of children.
* It wasn't very clear to us when exactly to acquire mutexes in our implementation. 
   * For example, when a function ends up calling other functions, sometimes the lock will be acquired in the caller and the lock is implicit in the callees, and other times it will be acquired in the callees.
* When `stop_elevator` is called for the first time, the `elevator_run` thread will be stopped and a new thread will be spawned for running `elevator_unload_all`.
   * Since unloading is a potentially long operation (~20 seconds), we do not want to block successive `stop_elevator` system calls, so we just put it in a thread and let it do its thing.
* When a request is issued with the same *start_floor* and *end_floor*, the passenger isn't entered into the queue and the passengers served for the floor is incremented.
   
### TODO
* ~~Handle fractional load units~~
* Handle `mutex_lock_interruptible` signals (i.e. when it doesn't return 0)
* Handle `floor_create` allocation failure
* Test thoroughly
