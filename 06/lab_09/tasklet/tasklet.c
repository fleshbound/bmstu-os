#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/atomic.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");

#define IRQ_NO 1
#define BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static struct tasklet_struct *tasklet = NULL;
static char *buffer = NULL;
static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;

static ssize_t my_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int my_release(struct inode*, struct file*);
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos);
static int my_open(struct inode *inode, struct file *file);

#ifdef HAVE_PROC_OPS
static struct proc_ops fops = {
    .proc_read = my_read,
    .proc_write = my_write,
    .proc_open = my_open,
    .proc_release = my_release
};
#else
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release
}
#endif

static ssize_t my_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "+ INFO: call my_read\n");
    return seq_read(file, buf, count, f_pos);
}

static int my_release(struct inode*, struct file*)
{
    printk(KERN_INFO "+ INFO: call my_release");
    return 0;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "+ INFO: call my_write");

    int space_left = BUF_SIZE + 1;

    if (space_left < count) {
        printk(KERN_ERR "+ ERROR: no space left\n");
        return -ENOSPC;
    }

    if (copy_from_user(buffer, buf, count)) {
        printk(KERN_ERR "+ ERROR: copy_from_user\n");
        return -EFAULT;
    }

    buffer[count - 1] = '\0';
    printk(KERN_INFO "+ INFO: success my_write");

    return count;
}

static int my_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "+ INFO: call my_show\n");

    seq_printf(m, "tasklet:\nstate - %ld\ncount - %d\nuse_callback - %d\ndata - %ld\n",
               tasklet->state, atomic_read(&tasklet->count), tasklet->use_callback, tasklet->data);

    return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "+ INFO: call my_open\n");
    return single_open(file, my_show, NULL);
}

void my_tasklet_fun(unsigned long data)
{
    printk(KERN_INFO "+ INFO: my_tasklet_func, arg=%c\n", (char)data);
}

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "+ INFO: call my_irq_handler");
    tasklet_schedule(tasklet);
    return IRQ_HANDLED;
}

static int __init my_init(void)
{
    int ret = request_irq(IRQ_NO, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler));

    if (ret)
    {
        printk(KERN_INFO "+ ERR: request_irq\n");
        return ret;
    }

    tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);

    if (!tasklet)
    {
        printk(KERN_INFO "+ ERR: kmalloc tasklet\n");
        free_irq(IRQ_NO, (void *)(my_irq_handler));
        return -ENOMEM;
    }

    tasklet_init(tasklet, my_tasklet_fun, 0);

    printk(KERN_INFO "+ INFO: my_init\n");

    buffer = (char *) vmalloc(BUF_SIZE);

    if (!buffer) {
        printk(KERN_ERR "+ ERR: can't vmalloc\n");
        return -ENOMEM;
    }

    memset(buffer, 0, BUF_SIZE);
    proc_file = proc_create_data("mytasklet", S_IRUGO | S_IWUGO, NULL, &fops, NULL);

    if (!proc_file) {
        vfree(buffer);
        printk(KERN_ERR "+ ERR: can't proc_create\n");
        return -ENOMEM;
    }
   
    printk(KERN_INFO "+ INFO: mytasklet_init");

    proc_dir = proc_mkdir("mytasklet_dir", NULL);
    proc_link = proc_symlink("mytasklet_symlink", NULL, "/proc/mytasklet");
    
    if (!proc_dir || !proc_link) {
        vfree(buffer);
        printk(KERN_ERR "+ ERROR: can't proc_mkdir or proc_symlink\n");
        return -ENOMEM;
    }

    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(tasklet);
    kfree(tasklet);
    free_irq(IRQ_NO, (void *)(my_irq_handler));
    printk(KERN_INFO "+ INFO: my_exit---------END\n");
}

module_init(my_init);
module_exit(my_exit);
