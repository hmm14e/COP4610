#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>

MODULE_LICENSE("GPL");

/* STUB pointers (were exported in the wrappers) that "register" the sys call functions */
extern long (*STUB_start_elevator) (void);
extern long (*STUB_issue_request) (int, int, int);
extern long (*STUB_stop_elevator) (void);

/* Implementation of the system calls, the STUB pointers will point to these functions */
long start_elevator(void)
{
    printk(KERN_INFO "starting the elevator service\n");
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


static int elevator_init(void)
{
    register_syscalls();
    return 0;
}


static void elevator_release(void)
{
    remove_syscalls();
}


module_init(elevator_init);
module_exit(elevator_release);
