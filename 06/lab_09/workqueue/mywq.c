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
#include <linux/time.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define IRQ_NO 1
#define BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

typedef struct {
    struct work_struct work;
    int code;
} my_work_t;

static struct workqueue_struct *mywq;
my_work_t *work_1, *work_2;

static char *ascii[84] =  { " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Backspace", 
                      "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl",
                      "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "\"", "'", "Shift (left)", "|", 
                      "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift (right)", 
                      "*", "Alt", "Space", "CapsLock", 
                      "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                      "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                      " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;

static ssize_t my_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int my_release(struct inode*, struct file*);
static int my_open(struct inode *inode, struct file *file);

#ifdef HAVE_PROC_OPS
static struct proc_ops fops = {
    .proc_read = my_read,
    .proc_open = my_open,
    .proc_release = my_release
};
#else
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
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

static int my_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "+ INFO: call my_show\n");

    seq_printf(m, "workqueue info:\n");

    return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "+ INFO: call my_open\n");
    return single_open(file, my_show, NULL);
}

static void mywq_fun_1(struct work_struct *work)
{
    my_work_t *mywork = (my_work_t *)work;

    if (mywork->code < 84)
        printk(KERN_INFO "+ mywq: mywq_fun_1: time = %llu, code = %s\n", ktime_get(), ascii[mywork->code]);
}

static void mywq_fun_2(struct work_struct *work)
{
    my_work_t *mywork = (my_work_t *)work;

    if ((mywork->code < 84) && (mywork->code >= 0))
    {
        printk(KERN_INFO "+ mywq: mywq_fun_2: time = %llu, delay = 1000\n", ktime_get());
        msleep(1000);
        printk(KERN_INFO "+ mywq: mywq_fun_2: time = %llu, code = %s\n", ktime_get(), ascii[mywork->code-128]);
    }
}

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    int code = inb(0x60);
    work_1->code = code;
    work_2->code = code;

    if (irq == IRQ_NO)
    {
        queue_work(mywq, (struct work_struct *)work_1);
        queue_work(mywq, (struct work_struct *)work_2);
        return IRQ_HANDLED;
    }

    printk(KERN_INFO "+ mywq: irq wasn't handled\n");
    return IRQ_NONE;
}

static int __init mywq_init(void)
{
    int ret = request_irq(IRQ_NO, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler));

    if (ret)
    {
        printk(KERN_ERR "+ mywq: request_irq\n");
        return ret;
    }

    mywq = alloc_workqueue("%s", __WQ_LEGACY | WQ_MEM_RECLAIM, 1, "my_queue");

    if (!mywq)
    {
        free_irq(IRQ_NO, (void *)(my_irq_handler));
        printk(KERN_ERR "+ mywq: create_workqueue\n");
        return -ENOMEM;
    }

    work_1 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (!work_1)
    {
        free_irq(IRQ_NO, (void *)(my_irq_handler));
        printk(KERN_ERR "+ mywq: kmalloc work_1\n");
        return -ENOMEM;
    }

    INIT_WORK((struct work_struct *)work_1, mywq_fun_1);

    work_2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (!work_2)
    {
        free_irq(IRQ_NO, (void *)(my_irq_handler));
        kfree(work_1);
        printk(KERN_ERR "+ mywq: kmalloc work_2\n");
        return -ENOMEM;
    }
    
    INIT_WORK((struct work_struct *)work_2, mywq_fun_2);

    proc_file = proc_create_data("tasklet", S_IRUGO, NULL, &fops, NULL);

    if (!proc_file) {
        printk(KERN_ERR "+ ERR: can't proc_create\n");
        return -ENOMEM;
    }
   
    printk(KERN_INFO "+ INFO: tasklet_init");

    proc_dir = proc_mkdir("tasklet_dir", NULL);
    proc_link = proc_symlink("tasklet_symlink", NULL, "/proc/tasklet");
    
    if (!proc_dir || !proc_link) {
        printk(KERN_ERR "+ ERROR: can't proc_mkdir or proc_symlink\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "+ myqw: init mywq\n");

    return 0;
}

static void __exit mywq_exit(void)
{
    proc_remove(proc_file);
    proc_remove(proc_dir);
    proc_remove(proc_link);
    flush_workqueue(mywq);
    destroy_workqueue(mywq);
    free_irq(IRQ_NO, (void *)(my_irq_handler));
    kfree(work_1);
    kfree(work_2);
    printk(KERN_INFO "+ mywq: exit mywq-----------------END");
}

module_init(mywq_init);
module_exit(mywq_exit);
