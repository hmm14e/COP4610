#include <linux/init.h>
#include <linux/delay.h>    /* ssleep */
#include <linux/kernel.h>
#include <linux/kthread.h>  /* kthread_run, kthread_stop */
#include <linux/module.h>   /* module_init, module_exit */
#include <linux/mutex.h>    /* mutex */
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
    struct mutex queue_lock;    /* ensure only one person modifying queue at once */
    int passengers_served;      /* total so far, **not including** people in queue */
    int floor_num;
    int load_in_weight;
    int load_in_units;
} Floor;

/* global variable that holds the array of `Floor`s */
Floor **floors;

Floor *floor_create(int floor_num)
{
    Floor *floor = kcalloc(1, sizeof(Floor), GFP_KERNEL);
    if (!floor)
        return NULL;
    INIT_LIST_HEAD(&floor->queue);
    floor->passengers_served = 0;
    floor->floor_num = floor_num;
    floor->load_in_weight = 0.0;
    floor->load_in_units = 0.0;
    return floor;
}

void floor_free(Floor* floor)
{
    /* clear the floor queue and free the struct */
    struct list_head *cur, *dummy;
    PassengerNode *passenger_node;
    /* free the linked list of passenger nodes from the floor queue */
    mutex_lock_interruptible(&floor->queue_lock);
    list_for_each_safe(cur, dummy, &floor->queue) {
        passenger_node = list_entry(cur, PassengerNode, queue);
        list_del(cur);
        kfree(passenger_node);
    }
    mutex_unlock(&floor->queue_lock);
    kfree(floor);
}


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

void floor_enqueue_passenger(Floor* floor, PassengerNode* p)
{
    mutex_lock_interruptible(&floor->queue_lock);
    floor->load_in_weight += PASSENGER_WEIGHTS[p->passenger_type];
    floor->load_in_units  += PASSENGER_UNITS[p->passenger_type];
    list_add_tail(&p->queue, &floor->queue);
    mutex_unlock(&floor->queue_lock);
}

PassengerNode* floor_dequeue_passenger(Floor* floor)
{
    mutex_lock_interruptible(&floor->queue_lock);
    mutex_unlock(&floor->queue_lock);
    return NULL;
}

void floor_print(Floor* floor)
{
    PassengerNode *p;
    if (list_empty(&floor->queue))
        return;
    mutex_lock_interruptible(&floor->queue_lock);
    printk("Floor %d: ", floor->floor_num);
    list_for_each_entry(p, &floor->queue, queue) {
        printk(KERN_CONT "{Type: %d, Dest: %d},  ", p->passenger_type, p->destination_floor);
    }
    mutex_unlock(&floor->queue_lock);
}

void print_floors_array(Floor** floors, int num_floors)
{
    int i;
    printk("Printing all floor queues\n");
    for (i = 0; i < num_floors; i++)
        floor_print(floors[i]);
    printk("------------------------------------------------------------------\n");
}

PassengerNode* floor_queue_front(Floor* floor){return NULL;}

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
    ElevatorState state;                    /* enum of possible states */
    int current_floor;
    int next_floor;
    int load_in_weight;              /* load in weight across all floors */
    int load_in_units;                      /* load in units across all floors */
    struct list_head passengers;            /* linked list of the passengers */
} Elevator;

static Elevator *elevator;                  /* global pointer to the elevator instance */
static struct task_struct *elevator_kthread; /* holds the pointer to the thread running the elevator */


Elevator* elevator_create(void)
{
    int i;
    Elevator * elv = kcalloc(1, sizeof(Elevator), GFP_KERNEL);
    elv->state = OFFLINE;
    elv->current_floor = 0;
    elv->next_floor = 0;
    elv->load_in_weight = 0.0;
    elv->load_in_units = 0.0;
    INIT_LIST_HEAD(&elv->passengers);
    return elv;
}

void elevator_free(Elevator* elv)
{
    /* at time of elevator_free, the elevator should be void of passengers
       but clear just in case
     */
    struct list_head *cur, *dummy;
    PassengerNode *passenger_node;
    /* free the linked list of passenger nodes from the elevator */
    list_for_each_safe(cur, dummy, &elv->passengers) {
        passenger_node = list_entry(cur, PassengerNode, queue);
        list_del(cur);
        kfree(passenger_node);
    }
    kfree(elv);
}

int elevator_run(void *arg)
{
    Elevator *elv = (Elevator*) arg;
    while (!kthread_should_stop()) {
        printk("elevator scanning\n");
        ssleep(30);
    }
    return 0;
}

int elevator_start(Elevator *elv)
{
    /* ...will return 1 if the elevator is already active, 0 for a successful elevator start */
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

void elevator_load_passenger(Elevator *elv, PassengerNode *p){}

void elevator_load_floor(Elevator *elv, Floor *floor){}

void elevator_unload_passengers(Elevator *elv, Floor *floor){}


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
    start_floor--; destination_floor--; /* requests are issued using 1-indexed vals */
    if ((start_floor < MIN_FLOOR || start_floor > MAX_FLOOR) ||
        (destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR)) {
        printk("issue_request: invalid floor value(s), start: %d, end: %d\n", start_floor, destination_floor);
        return 1;
    }
    printk(KERN_DEBUG "issuing a request type: %d, start: %d, dest: %d\n", passenger_type, start_floor, destination_floor);
    PassengerNode *p = passenger_node_create(passenger_type, destination_floor);
    if (!p)
        return 1;
    floor_enqueue_passenger(floors[start_floor], p);
    print_floors_array(floors, NUM_FLOORS);
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
