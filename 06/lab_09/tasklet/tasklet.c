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
#include <linux/version.h>
#include <linux/time.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#define IRQ_NO 1
#define BUF_SIZE PAGE_SIZE

char * ascii[84] =  { " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Backspace", 
                      "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl",
                      "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "\"", "'", "Shift (left)", "|", 
                      "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift (right)", 
                      "*", "Alt", "Space", "CapsLock", 
                      "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                      "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                      " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

static struct tasklet_struct *tasklet = NULL;
static char *buffer = NULL;

void my_tasklet_fun(unsigned long data)
{
    int code = inb(0x60);

    if (code < 84)
        printk(KERN_INFO "+ INFO: call my_tasklet_func,    tasklet state=%ld, time=%llu, inb=%s\n", tasklet->state, ktime_get(), ascii[code]);
}

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    if (irq == IRQ_NO)
    {
        printk(KERN_INFO "+ INFO: before tasklet_schedule, tasklet state=%ld, time=%llu\n", tasklet->state, ktime_get());
        tasklet_schedule(tasklet);
        printk(KERN_INFO "+ INFO: after tasklet_schedule,  tasklet state=%ld, time=%llu\n", tasklet->state, ktime_get());
        return IRQ_HANDLED;
    }

    printk(KERN_INFO "+ INFO: irq wasn't handled\n");
    return IRQ_NONE;
}

static int __init my_init(void)
{
    int ret = request_irq(IRQ_NO, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler));

    if (ret)
    {
        printk(KERN_ERR "+ ERR: request_irq\n");
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
