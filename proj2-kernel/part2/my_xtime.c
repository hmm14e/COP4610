#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("DUAL BSD/GPL");

static int my_xtime_init(void)
{
    printk(KERN_ALERT "my_xtime_init called\n");
    return 0;
}

static void my_xtime_exit(void)
{
    printk(KERN_ALERT "my_xtime_exit called\n");
}

module_init(my_xtime_init);
module_exit(my_xtime_exit);

