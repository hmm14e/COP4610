#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("DUAL BSD/GPL");
MODULE_DESCRIPTION("Module featuring proc read");


#define PROC_NAME "timed"
#define PROC_PERMS 0644
#define PROC_PARENT_DIR NULL
#define MESSAGE_BUFFER_SIZE 512

static struct file_operations fops; /* proc file operaitons */
static char *message;               /* message to display in proc */
static int read_p;                  /* no idea what this is for  */
static struct timespec prev_timespec, curr_timespec, diff_timespec;



struct timespec timespec_diff(struct timespec start, struct timespec end)
{
    struct timespec result;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = end.tv_sec - start.tv_sec - 1;
        result.tv_nsec = end.tv_nsec - start.tv_nsec + 1000000000;
    }
    else {
        result.tv_sec = end.tv_sec - start.tv_sec;
        result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return result;
}

int xtime_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    read_p = 1;
    message = kcalloc(MESSAGE_BUFFER_SIZE, sizeof(char),  __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    if (!message) {
        printk(KERN_WARNING "xtime_proc_read error\n");
        return -ENOMEM;
    }
    return 0;
}


ssize_t xtime_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{       
    curr_timespec = current_kernel_time();
    
    read_p = !read_p;
    if (read_p)
        return 0;
    int len = snprintf(message, MESSAGE_BUFFER_SIZE, "current time: %lld.%.9ld\n",
                      (long long) curr_timespec.tv_sec, curr_timespec.tv_nsec);
    /* on first call prev_timespec.tv_sec is 0 */
    if (prev_timespec.tv_sec != 0) {
        diff_timespec = timespec_diff(prev_timespec, curr_timespec);
        len += snprintf(message + len, MESSAGE_BUFFER_SIZE - len, "elapsed time: %lld.%.9ld\n",
                       (long long) (diff_timespec.tv_sec), diff_timespec.tv_nsec);
    }

    prev_timespec.tv_sec = curr_timespec.tv_sec;
    prev_timespec.tv_nsec = curr_timespec.tv_nsec;
    copy_to_user(buf, message, len);
    return len;
}

int xtime_proc_release(struct inode *sp_inode, struct file *sp_file)
{
    printk(KERN_INFO "xtime_proc_release called\n");
    kfree(message);
    return 0;
}


static int xtime_init(void)
{
    printk("xtime_init called\n");
    fops.open = xtime_proc_open;
    fops.read = xtime_proc_read;
    fops.release = xtime_proc_release;
    
    /* init to 0 to denote the first read */
    prev_timespec.tv_sec = 0;
    
    if (!proc_create(PROC_NAME, PROC_PERMS, PROC_PARENT_DIR, &fops)) {
        printk("ERROR: proc_create");
        remove_proc_entry(PROC_NAME, NULL);
        return -ENOMEM;
    }

    return 0;
}

static void xtime_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk("xtime_exit called\n");
}

module_init(xtime_init);
module_exit(xtime_exit);

