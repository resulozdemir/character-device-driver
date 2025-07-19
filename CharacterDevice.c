#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/slab.h> 
#include <linux/timer.h>

#define MAX_SIZE 1000
static char inp[MAX_SIZE];

static dev_t resul_dev;
static struct cdev resul_cdev;
static struct class *resul_class;

static DECLARE_WAIT_QUEUE_HEAD(resul_wq);
static struct timer_list my_timer;
static int timer_expired = 0;

#define IOCTL_GET_SIZE 	    _IOR('X', 0, int)
#define IOCTL_CLEAR_BUFFER  _IO('X', 1)
#define IOCTL_REVERSE_DATA  _IO('X', 2)

void my_timer_callback(struct timer_list *t) {
     timer_expired = 1; 
     wake_up_interruptible(&resul_wq);
     printk(KERN_INFO "resul: my_timer_callback()\n");
}

ssize_t resul_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "resul: write()\n");   

    if (copy_from_user(inp, buf, count))
        return -EFAULT;
    
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + 10*HZ);
    
    return count;	
}
					
ssize_t resul_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "resul: read()\n");

    if (copy_to_user(buf, inp, count))
        return -EFAULT;

    return count;
}

unsigned int resul_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    printk(KERN_INFO "resul: poll()\n");

    poll_wait(filp, &resul_wq, wait);

    if(timer_expired)
    	mask |= POLLIN | POLLRDNORM; 
    	
    return mask;
}

long resul_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_GET_SIZE:
        {
            int size = sizeof(inp);
            if(copy_to_user((int __user *)arg, &size, sizeof(int)))
                return -EFAULT;
            break;
        }
        case IOCTL_CLEAR_BUFFER:
            memset(inp, 0, sizeof(inp));
            break;
        case IOCTL_REVERSE_DATA:
        {
    		int i, j;
   		char temp;
   		int len = strlen(inp);
    		for (i = 0, j = len - 1; i < j; i++, j--) {
     		   temp = inp[i];
     		   inp[i] = inp[j];
    		   inp[j] = temp;
   		}

    		if (copy_to_user((char __user *)arg, inp, MAX_SIZE))
        		return -EFAULT;

   	    	break;
	}
        default:
            return -ENOTTY;
    }
    return 0;
}

static struct file_operations resul_fops = {
    .owner = THIS_MODULE,
    .write = resul_write,
    .read = resul_read,
    .poll = resul_poll,
    .unlocked_ioctl = resul_ioctl,
};

static int __init resul_init(void)
{
    int rtn;

    printk(KERN_INFO "resul: init()\n");

    rtn = alloc_chrdev_region(&resul_dev, 0, 1, "resul");
    if (rtn < 0) {
        printk(KERN_INFO "Unable to allocate major number for resul\n");
        return rtn;
    }

    cdev_init(&resul_cdev, &resul_fops);
    rtn = cdev_add(&resul_cdev, resul_dev, 1);
    if (rtn < 0) {
        unregister_chrdev_region(resul_dev, 1);
        printk(KERN_INFO "Unable to add cdev for resul\n");
        return rtn;
    }

    resul_class = class_create(THIS_MODULE, "resul");
    if (IS_ERR(resul_class)) {
        printk(KERN_ERR "Error creating resul class.\n");
        cdev_del(&resul_cdev);
        unregister_chrdev_region(resul_dev, 1);
        return PTR_ERR(resul_class);
    }
    
    if (!device_create(resul_class, NULL, resul_dev, NULL, "resul")) {
        printk(KERN_ERR "Error creating resul device.\n");
        class_destroy(resul_class);
        cdev_del(&resul_cdev);
        unregister_chrdev_region(resul_dev, 1);
        return -1;
    }

    return 0;
}

static void __exit resul_exit(void)
{
    printk(KERN_INFO "resul: exit()\n");
    del_timer(&my_timer);
    device_destroy(resul_class, resul_dev);
    class_destroy(resul_class);
    cdev_del(&resul_cdev);
    unregister_chrdev_region(resul_dev, 1);
}

module_init(resul_init);
module_exit(resul_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Resul");