#include <linux/init.h>
#include <linux/delay.h>    /* ssleep */
#include <linux/kernel.h>
#include <linux/module.h>   /* module_init, module_exit */
#include <linux/mutex.h>    /* mutex */
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/slab.h>     /* kmalloc, kfree */
#include <linux/proc_fs.h>  /* proc_create, fops */


MODULE_LICENSE("GPL");

#define MIN_FLOOR 0             /* min floor number, default position */
#define MAX_FLOOR 9             /* max floor number (inclusive) */
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
} Passenger_type;

typedef struct {
    int passenger_type;
    int destination_floor;
} Passenger;

static float PASSENGER_UNITS[4] = {1, 1, 2, 2};
static float PASSENGER_WEIGHTS[4] = {1, 0.5, 2, 3};

/**
 ************************************************************************************
 ***************************** Floor Implementation *********************************
 ************************************************************************************
 */

typedef struct {
    struct list_head queue; /* pointer to previous and next `PassengerNode` */
    int passenger_type;
    int destination_floor;
} PassengerNode;

typedef struct {
    struct list_head queue;     /* queue that holds the passengers (via `PassengerNodes` */
    struct mutex queue_lock;    /* ensure only one person modifying queue at once */
    int passengers_served;      /* total so far, **not including** people in queue */
    int load_in_weight;
    int load_in_units;
} Floor;

Floor* floor_create(void){return NULL;}

void floor_enqueue_passenger(Passenger* p){}

Passenger* floor_dequeue_passenger(void){return NULL;}

Passenger* floor_queue_front(void){return NULL;}


/**
 ************************************************************************************
 **************************** Elevator Implementation *******************************
 ************************************************************************************
 */
typedef enum {
    OFFLINE, /* elevator isn't running but the module is loaded (initial state) */
    IDLE,    /* elevator is stopped on a floor because there are no more passengers to service */
    LOADING, /* elevator is stopped on a floor to load and unload passengers */
    UP,      /* elevator is moving from a lower floor to a higher floor */
    DOWN     /* elevator is moving from a higher floor to a lower floor */
} ElevatorState;


typedef struct {
    ElevatorState state;   /* enum */
    int current_floor;
    int next_floor;
    float load_in_weight;
    float load_in_units;
    int passengers[5];      /* TODO: find a good easy way to store passengers */
} Elevator;

Elevator* elevator_create(void){return NULL;}

int elevator_start(Elevator *elv){return 0;}

void elevator_move(Elevator *elv, int floor_num){}

void elevator_load_passenger(Elevator *elv, Passenger *p){}

void elevator_load_floor(Elevator *elv, Floor *floor){}

void elevator_unload_passengers(Elevator *elv, Floor *floor){}


/**
 ************************************************************************************
 **************************** Controller Implementation *****************************
 ************************************************************************************
 */

/* nothing here is set in stone */

void controller_add_request(int passenger_type, int start_floor, int destination_floor){}

void controller_schedule(Elevator *elv){}


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
    /*  */
    return 0;
}


long issue_request(int passenger_type, int start_floor, int destination_floor)
{
    printk(KERN_INFO "issuing a request to the elevator\n");
    return 0;
}

long stop_elevator(void)
{
    printk(KERN_INFO "stopping the elevator service\n");
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


    return 0;
}


static void elevator_module_exit(void)
{
    remove_syscalls();
}


module_init(elevator_module_init);
module_exit(elevator_module_exit);
