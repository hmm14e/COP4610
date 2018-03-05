#include <linux/init.h>
#include <linux/delay.h>    /* ssleep */
#include <linux/kernel.h>
#include <linux/kthread.h>  /* kthread_run, kthread_stop */
#include <linux/module.h>   /* module_init, module_exit */
#include <linux/mutex.h>    /* mutex */
#include <linux/sched.h>    /* schedule */
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/slab.h>     /* kmalloc, kfree */
#include <linux/proc_fs.h>  /* proc_create, fops */


MODULE_LICENSE("GPL");

#define MIN_FLOOR 0             /* min floor number, default position */
#define MAX_FLOOR 9             /* max floor number (inclusive) */
#define NUM_FLOORS 10           /* */
#define TIME_BETWEEN_FLOORS 2   /* mandatory time spent moving between floors */
#define TIME_AT_FLOOR 2         /* mandatory time spent loading/unloading */
#define MAX_LOAD_UNITS 10       /* max load the elevator can hold in terms on units */
#define MAX_LOAD_WEIGHT 15      /* max load the elevator can hold in terms on weight */


/**
 ************************************************************************************
 **************************** Passenger Implementation ******************************
 ************************************************************************************
 */
typedef enum {
    CHILD,
    ADULT,
    BELLHOP,
    ROOM_SERVICE
} PassengerType;


typedef struct {
    struct list_head queue; /* list of all the passengers */
    int passenger_type;
    int destination_floor;
} PassengerNode;

static int PASSENGER_UNITS[4] = {1, 1, 2, 2};
static int PASSENGER_WEIGHTS[4] = {1, 1, 2, 3};

PassengerNode *passenger_node_create(PassengerType passenger_type, int destination_floor)
{
    PassengerNode *p = kcalloc(1, sizeof(PassengerNode), GFP_KERNEL);
    if (!p) {
        printk("passenger_node_create: failed to kcalloc passenger\n");
        return NULL;
    }
    p->passenger_type = passenger_type;
    p->destination_floor = destination_floor;
    return p;


}

/**
 ************************************************************************************
 ***************************** Floor Implementation *********************************
 ************************************************************************************
 */

typedef struct {
    struct list_head queue;     /* queue that holds the passengers (via `PassengerNodes`)*/
    struct mutex lock;    /* ensure only one person modifying queue at once */
    int passengers_served;      /* total so far, **not including** people in queue */
    int floor_num;
    int load_in_weight;
    int load_in_units;
} Floor;

/* global variable that holds the array of `Floor`s */
Floor **floors;

/* initalizes `Floor` struct with everything zero'd out */
Floor *floor_create(int floor_num)
{
    Floor *floor = kcalloc(1, sizeof(Floor), GFP_KERNEL);
    if (!floor)
        return NULL;
    INIT_LIST_HEAD(&floor->queue);
    floor->floor_num = floor_num;
    return floor;
}

/* deep free of the floors struct */
void floor_free(Floor* floor)
{
    /* clear the floor queue and free the struct */
    struct list_head *cur, *dummy;
    PassengerNode *passenger_node;
    /* free the linked list of passenger nodes from the floor queue */
    mutex_lock_interruptible(&floor->lock);
    list_for_each_safe(cur, dummy, &floor->queue) {
        passenger_node = list_entry(cur, PassengerNode, queue);
        list_del(cur);
        kfree(passenger_node);
    }
    mutex_unlock(&floor->lock);
    kfree(floor);
}

/* dyanically allocate space for the global array holding all the floors */
Floor** create_floors_array(int num_floors)
{
    int i;
    Floor **floors = kcalloc(num_floors, sizeof(Floor*), GFP_KERNEL);
    if (!floors)
        return NULL;
    /* TODO: handle failed allocation of a floor */
    for (i = 0; i < num_floors; i++)
        floors[i] = floor_create(i);
    return floors;
}


void free_floors_array(Floor** floors, int num_floors)
{
    int i;
    for (i = 0; i < num_floors; i++) {
        floor_free(floors[i]);
    }
    kfree(floors);
}

/**
 * adds passenger to the end of the floor queue, updating the floor load metrics
 */
void floor_enqueue_passenger(Floor* floor, PassengerNode* p)
{
    mutex_lock_interruptible(&floor->lock);
    floor->load_in_weight += PASSENGER_WEIGHTS[p->passenger_type];
    floor->load_in_units  += PASSENGER_UNITS[p->passenger_type];
    list_add_tail(&p->queue, &floor->queue);
    mutex_unlock(&floor->lock);
}

void floor_print(Floor* floor)
{
    PassengerNode *p;
    if (list_empty(&floor->queue))
        return;
    mutex_lock_interruptible(&floor->lock);
    printk("Floor %d: ", floor->floor_num);
    list_for_each_entry(p, &floor->queue, queue) {
        printk(KERN_CONT "{Type: %d, Dest: %d},  ", p->passenger_type, p->destination_floor);
    }
    mutex_unlock(&floor->lock);
}


void print_floors_array(Floor** floors, int num_floors)
{
    int i;
    printk("Printing all floor queues\n");
    for (i = 0; i < num_floors; i++)
        floor_print(floors[i]);
    printk("------------------------------------------------------------------\n");
}


/**
 ************************************************************************************
 **************************** Elevator Implementation *******************************
 ************************************************************************************
 */
typedef enum {
    OFFLINE,    /* elevator isn't running but the module is loaded (initial state) */
    IDLE,       /* elevator is stopped on a floor because there are no more passengers to service */
    LOADING,    /* elevator is stopped on a floor to load and unload passengers */
    UP,         /* elevator is moving from a lower floor to a higher floor */
    DOWN        /* elevator is moving from a higher floor to a lower floor */
} ElevatorState;


typedef struct {
    struct list_head queue;     /* queue of the passengers */
    struct mutex lock;    /* lock to stop the elevator from being modified */
    ElevatorState state;        /* enum of possible states */
    int current_floor;
    int next_floor;
    int num_passengers;
    int load_in_weight;
    int load_in_units;
} Elevator;

static Elevator *elevator;                      /* global pointer to the elevator instance */
static struct task_struct *elevator_kthread;    /* holds the pointer to the thread running the elevator */

/* return an allocated and initialized elevator */
Elevator* elevator_create(void)
{
    Elevator * elv = kcalloc(1, sizeof(Elevator), GFP_KERNEL);
    elv->state = OFFLINE;
    INIT_LIST_HEAD(&elv->queue);
    return elv;
}



/* step one floor in the `direction` */
void elevator_step(Elevator *elv, ElevatorState direction)
{
    int delta = direction == UP ? 1 : -1;
    mutex_lock_interruptible(&elv->lock);
    elv->state = direction;
    elv->current_floor = elv->next_floor;
    elv->next_floor = elv->current_floor + delta;
    mutex_unlock(&elv->lock);
    ssleep(TIME_BETWEEN_FLOORS);
}

/* move to `dest_floor` without servicing anybody on the way */
void elevator_move_to(Elevator *elv, int dest_floor)
{
    int delta = dest_floor > elv->current_floor ? 1 : -1;
    mutex_lock_interruptible(&elv->lock);
    elv->state = dest_floor > elv->current_floor ? UP : DOWN;
    elv->next_floor = dest_floor; /* next floor to *service* */
    mutex_unlock(&elv->lock);
    while (elv->current_floor != dest_floor) {
        /* switch off locking and unlocking because moving between floors is very slow */
        mutex_lock_interruptible(&elv->lock);
        elevator->current_floor += delta;
        mutex_unlock(&elv->lock);
        ssleep(TIME_BETWEEN_FLOORS);
    }
}


/**
 * Reset the scan direction and next floor based on the first passenger waiting on the elevators current floor
 * NOTE: this function requires that a the current floor contains at least one passenger
 */
void elevator_setup_scan(Elevator *elv)
{
    PassengerNode *p = list_first_entry(&floors[elv->current_floor]->queue, PassengerNode, queue);
    mutex_lock_interruptible(&elv->lock);
    elv->state = p->destination_floor > elv->current_floor ? UP : DOWN;
    elv->next_floor = elv->state == UP ? elv->current_floor + 1 : elv->current_floor - 1;
    mutex_unlock(&elv->lock);
}


/**
 * coming from IDLE state, try to find a floor that has someone waiting, move there, and start a scan to their dest
 * if no one is waiting, maintain the IDLE state and put the thread to sleep via `schedule`
 */
void elevator_try_to_start_scan(Elevator *elv)
{
    int i;
    for (i = 0; i <= MAX_FLOOR; i++){
        if (!list_empty(&floors[i]->queue)) {
            elevator_move_to(elv, i);
            elevator_setup_scan(elv);
            return;
        }
    }
    schedule();
}

/**
 * picks up as many people moving in the same direciton as possible
 * NOTE: spec mandates that the elevator must pick up people heading in the same direction
 */
void elevator_load_passengers(Elevator *elv)
{
    PassengerNode *p; /* person at front of line */
    int can_fit, same_direction, p_weight, p_units;
    Floor *floor = floors[elv->current_floor];

    mutex_lock_interruptible(&floor->lock);
    mutex_lock_interruptible(&elv->lock);
    elv->state = LOADING;
    while (!list_empty(&floor->queue)) {
        p = list_first_entry(&floor->queue, PassengerNode, queue);
        p_weight = PASSENGER_WEIGHTS[p->passenger_type];
        p_units = PASSENGER_UNITS[p->passenger_type];
        /* TODO: handle fractional weights and units */
        can_fit = ((elv->load_in_weight + p_weight <= MAX_LOAD_WEIGHT) &&
                   (elv->load_in_units + p_units <= MAX_LOAD_UNITS));
        same_direction = ((elv->state == UP && p->destination_floor > elv->current_floor) ||
                          (elv->state == DOWN && p->destination_floor < elv->current_floor));

        if (!can_fit || !same_direction)
            break;

        /* remove person from the floor queue and put them into the elevator queue */
        list_del(&p->queue);
        floor->passengers_served++;

        list_add_tail(&p->queue, &elv->queue);
        elv->num_passengers++;
        elv->load_in_weight += p_weight;
        elv->load_in_units += p_units;
    }
    mutex_unlock(&elv->lock);
    mutex_unlock(&floor->lock);
}


/**
 * remove and free all passengers that are at their destination
 */
void elevator_unload_passengers(Elevator *elv)
{
    struct list_head *cur, *dummy;
    PassengerNode *p;
    mutex_lock_interruptible(&elv->lock);
    elv->state = LOADING;
    /* remove passengers at their desitnation */
    list_for_each_safe(cur, dummy, &elv->queue) {
        p = list_entry(cur, PassengerNode, queue);
        if (p->destination_floor == elv->current_floor){
            list_del(cur);
            elv->num_passengers--;
            elv->load_in_weight -= PASSENGER_WEIGHTS[p->passenger_type];
            elv->load_in_units -= PASSENGER_UNITS[p->passenger_type];
            kfree(p);
        }
    }
    mutex_unlock(&elv->lock);
}



/**
 * services passengers via the SCAN algrotihm (this runs in a separate thread)
 */
int elevator_run(void *args)
{
    ElevatorState direction;
    Elevator *elv = (Elevator*) args; /* `kthread_run` passes in pointer of the args */
    while (!kthread_should_stop()) {
        if (elv->state == IDLE) {
            /* look for someone to service or sleep if there is no-one */
            elevator_try_to_start_scan(elv);
        }
        else {
            direction = elv->state; /* save direction */
            elevator_unload_passengers(elv);
            if (elv->num_passengers == 0){
                /* once the car is empty, we have finished our current scan and can start a new one */
                elv->state = IDLE;
            }
            else {
                /* continue the current scan */
                elevator_load_passengers(elv);
                elevator_step(elv, direction);
            }
        }
    }
    return 0;
}


/**
 * start the elevator by spawning a thread to scan the floors
 * @return: 1 if the elevator is already active, 0 for a successful elevator start
 */
int elevator_start(Elevator *elv)
{
    if (elv->state != OFFLINE)
        return 1;
    elv->state = IDLE;
    /* spawn a thread to `run` the elevator */
    elevator_kthread = kthread_run(elevator_run, elv, "elevator_run");
    if (IS_ERR(elevator_kthread)) {
        printk("ERROR: kthread_run(elevator_run, ...)\n");
        return PTR_ERR(elevator_kthread);
    }
    return 0;
}



/**
 * free the elevator `elv`, making sure to free all passengers in its queue
 * NOTE: by the time this is called, the elevator should have unloaded all passengers
 */
void elevator_free(Elevator* elv)
{
    /* at time of elevator_free, the elevator should be void of passengers
       but clear just in case
     */
    struct list_head *cur, *dummy;
    PassengerNode *passenger_node;
    /* free the linked list of passenger nodes from the elevator */
    list_for_each_safe(cur, dummy, &elv->queue) {
        passenger_node = list_entry(cur, PassengerNode, queue);
        list_del(cur);
        kfree(passenger_node);
    }
    kfree(elv);
}



/**
 ************************************************************************************
 ********************************* Syscall Functions ********************************
 ************************************************************************************
 */

/* STUB pointers (were exported in the wrappers) that "register" the sys call functions */
extern long (*STUB_start_elevator) (void);
extern long (*STUB_issue_request) (int, int, int);
extern long (*STUB_stop_elevator) (void);


/* Implementation of the system calls, the STUB pointers will point to these functions */
long start_elevator(void)
{
    printk(KERN_INFO "starting the elevator service\n");
    /* allocate and start the global elevator */
    return elevator_start(elevator);
}


long issue_request(int passenger_type, int start_floor, int destination_floor)
{
    PassengerNode *p;
    start_floor--; destination_floor--; /* requests are issued using 1-indexed vals */
    if ((start_floor < MIN_FLOOR || start_floor > MAX_FLOOR) ||
        (destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR)) {
        printk("issue_request: invalid floor value(s), start: %d, end: %d\n", start_floor, destination_floor);
        return 1;
    }
    else if (start_floor == destination_floor) {
        /* don't even bother enqueueing if the passenger doesn't need to go anywhere */
        floors[start_floor]->passengers_served++;
    }
    else {
        p = passenger_node_create(passenger_type, destination_floor);
        if (!p)
            return 1;
        floor_enqueue_passenger(floors[start_floor], p);
        print_floors_array(floors, NUM_FLOORS);
    }
    return 0;

}

long stop_elevator(void)
{
    printk(KERN_INFO "stopping the elevator service\n");
    kthread_stop(elevator_kthread);
    return 0;
}

static void register_syscalls(void)
{
    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;
}


static void remove_syscalls(void)
{
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;
}


/**
 ************************************************************************************
 ********************************* Procfs Functions *********************************
 ************************************************************************************
 */

static struct file_operations fops; /* proc file operaitons */
static char *message;               /* message to display in proc */
static int read_p;                  /* no idea what this is for  */


int elevator_proc_open(struct inode *sp_inode, struct file *sp_file) {
    read_p = 1;
    message = kcalloc(256, sizeof(char), __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    if (message == NULL) {
        printk(KERN_WARNING "elevator_proc_open");
        return -ENOMEM;
    }
    return 0;
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
    int len = strlen(message);

    read_p = !read_p;
    if (read_p)
        return 0;

    copy_to_user(buf, message, len);
    return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file) {
    kfree(message);
    return 0;
}

/**
 ************************************************************************************
 ********************************* Module Functions *********************************
 ************************************************************************************
 */


static int elevator_module_init(void)
{
    register_syscalls();
    floors = create_floors_array(NUM_FLOORS);
    elevator = elevator_create();
    if (!floors)
        return -ENOMEM; /* no space available? */
    return 0;
}


static void elevator_module_exit(void)
{
    free_floors_array(floors, NUM_FLOORS);
    elevator_free(elevator);
    remove_syscalls();
}


module_init(elevator_module_init);
module_exit(elevator_module_exit);
