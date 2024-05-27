#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/init_task.h>  
#include <linux/kernel.h>     
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/path.h>
#include <linux/kstrtox.h>

MODULE_LICENSE("GPL");

#define COOKIE_BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static ssize_t myseq_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int myseq_release(struct inode*, struct file*);
static int myseq_open(struct inode *inode, struct file *file);

#ifdef HAVE_PROC_OPS
static struct proc_ops fops = {
    .proc_read = myseq_read,
    .proc_open = myseq_open,
    .proc_lseek  = seq_lseek,
    .proc_release = myseq_release
};
#else
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = myseq_read,
    .open = myseq_open,
    .llseek  = seq_lseek,
    .release = myseq_release
}
#endif

void *myseq_start(struct seq_file *m, loff_t *pos);
void myseq_stop(struct seq_file *m, void *v);
void *myseq_next(struct seq_file *m, void *v, loff_t *pos);
int myseq_show(struct seq_file *m, void *v);

static struct seq_operations seq_ops = {
    .start = myseq_start,
    .stop = myseq_stop,
    .next = myseq_next,
    .show = myseq_show,
};

static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;
static int limit = 1;

void *myseq_start(struct seq_file *m, loff_t *pos)
{
    printk(KERN_INFO "** INFO: call myseq_start\n");

    if (*pos >= limit)
    {
        printk(KERN_INFO "+ INFO: done\n");
        return NULL;
    }

    loff_t *spos = kmalloc(sizeof(loff_t), GFP_KERNEL);
    seq_printf(m, "current:\ncomm - %s\npid - %d\nparent comm - %s\nppid - %d\nstate - %d\non_cpu - %d\nflags - %x\nprio - %d\npolicy - %d\nexit_state - %d\nexit_code - %d\nin_execve - %x\nutime - %llu\nroot - %s\n",
            current->comm,
            current->pid,
            current->parent->comm,
            current->parent->pid,
            current->__state,
            current->on_cpu,
            current->flags,
            current->prio,
            current->policy,
            current->exit_state,
            current->exit_code,
            current->in_execve,
            current->utime,
            current->fs->root.dentry->d_name.name);

    if (!spos)
        return NULL;

    *spos = *pos;
    return spos;
}

void myseq_stop(struct seq_file *m, void *v)
{
    printk(KERN_INFO "** INFO: call myseq_stop\n");
    kfree(v);
}

void *myseq_next(struct seq_file *m, void *v, loff_t *pos)
{
    printk(KERN_INFO "** INFO: call myseq_next\n");
    loff_t *spos = v; 

    *pos = ++*spos;

    if (*pos >= limit)
        return NULL;
    
    return spos;
}

int myseq_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "** INFO: call myseq_show\n");
    loff_t *spos = v;
    seq_printf(m, "%lld\n", (long long) *spos);
    return 0;
}

static ssize_t myseq_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "** INFO: call myseq_read\n");
    return seq_read(file, buf, count, f_pos);
}

static int myseq_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "** INFO: call myseq_open\n");
    return seq_open(file, &seq_ops);
}

static int myseq_release(struct inode*, struct file*)
{
    printk(KERN_INFO "** INFO: call myseq_release\n");
    return 0;
}

static int __init myseq_init(void)
{
    proc_file = proc_create_data("myseq", S_IRUGO, NULL, &fops, NULL);

    if (!proc_file) {
        printk(KERN_ERR "** ERROR: can't proc_create\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "** INFO: myseq_init");
    proc_dir = proc_mkdir("myseq_dir", NULL);
    proc_link = proc_symlink("myseq_symlink", NULL, "/proc/myseq");
    
    if (!proc_dir || !proc_link) {
        printk(KERN_ERR "** ERROR: can't proc_mkdir or proc_symlink\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "** INFO: myseq module loaded\n");

    return 0;
}

static void __exit myseq_exit(void)
{
    proc_remove(proc_file);
    proc_remove(proc_dir);
    proc_remove(proc_link);
    printk(KERN_INFO "** INFO: myseq module unloaded----------END-----------\n");
}

module_init(myseq_init);
module_exit(myseq_exit);
