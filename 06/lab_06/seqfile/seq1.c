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

#define BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static ssize_t myseq_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int myseq_release(struct inode*, struct file*);
static ssize_t myseq_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos);
static int myseq_open(struct inode *inode, struct file *file);

#ifdef HAVE_PROC_OPS
static struct proc_ops fops = {
    .proc_read = myseq_read,
    .proc_write = myseq_write,
    .proc_open = myseq_open,
    .proc_release = myseq_release
};
#else
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = myseq_read,
    .write = myseq_write,
    .open = myseq_open,
    .release = myseq_release
}
#endif

static char *buffer = NULL;
static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;

static ssize_t myseq_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "** INFO: call myseq_read\n");
    return seq_read(file, buf, count, f_pos);
}

static int myseq_release(struct inode*, struct file*)
{
    printk(KERN_INFO "** INFO: call myseq_release");
    return 0;
}

static ssize_t myseq_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "** INFO: call myseq_write");

    int space_left = BUF_SIZE + 1;

    if (space_left < count) {
        printk(KERN_ERR "** ERROR: no space left\n");
        return -ENOSPC;
    }

    if (copy_from_user(buffer, buf, count)) {
        printk(KERN_ERR "** ERROR: copy_from_user\n");
        return -EFAULT;
    }

    buffer[count - 1] = '\0';
    printk(KERN_INFO "** INFO: success myseq_write");

    return count;
}

static int myseq_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "** INFO: call myseq_show\n");

    int pid;

    if (kstrtoint(buffer, 0, &pid))
    {
        seq_printf(m, "%s: pid must be an integer\n", buffer);
        return 0;
    }

    struct task_struct *task = get_pid_task(find_get_pid((pid_t)pid), PIDTYPE_PID);
    
    if (!task)
    {
        seq_printf(m, "%d: no such task\n", pid);
        return 0;
    }

    seq_printf(m, "%d:\ncomm - %s\npid - %d\nparent comm - %s\nppid - %d\nstate - %d\non_cpu - %d\nflags - %x\nprio - %d\npolicy - %d\nexit_state - %d\nexit_code - %d\nin_execve - %x\nutime - %llu\nroot - %s\n",
            pid,
            task->comm,
            task->pid,
            task->parent->comm,
            task->parent->pid,
            task->__state,
            task->on_cpu,
            task->flags,
            task->prio,
            task->policy,
            task->exit_state,
            task->exit_code,
            task->in_execve,
            task->utime,
            task->fs->root.dentry->d_name.name);

    return 0;
}

static int myseq_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "** INFO: call myseq_open\n");
    return single_open(file, myseq_show, NULL);
}

static int __init myseq_init(void)
{
    buffer = (char *) vmalloc(BUF_SIZE);

    if (!buffer) {
        printk(KERN_ERR "** ERROR: can't vmalloc\n");
        return -ENOMEM;
    }

    memset(buffer, 0, BUF_SIZE);
    proc_file = proc_create_data("myseq", S_IRUGO | S_IWUGO, NULL, &fops, NULL);

    if (!proc_file) {
        vfree(buffer);
        printk(KERN_ERR "** ERROR: can't proc_create\n");
        return -ENOMEM;
    }
   
    printk(KERN_INFO "** INFO: myseq_init");

    proc_dir = proc_mkdir("myseq_dir", NULL);
    proc_link = proc_symlink("myseq_symlink", NULL, "/proc/myseq");
    
    if (!proc_dir || !proc_link) {
        vfree(buffer);
        printk(KERN_ERR "** ERROR: can't proc_mkdir or proc_symlink\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "** INFO: myseq module loaded\n");

    return 0;
}

static void __exit myseq_exit(void) {
    proc_remove(proc_file);
    proc_remove(proc_dir);
    proc_remove(proc_link);
    vfree(buffer);
    printk(KERN_INFO "** INFO: myseq module unloaded----------END-----------\n");
}

module_init(myseq_init);
module_exit(myseq_exit);
