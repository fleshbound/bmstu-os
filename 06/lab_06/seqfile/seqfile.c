#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");

#define COOKIE_BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int fortune_release(struct inode*, struct file*);
static ssize_t fortune_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos);
static int fortune_open(struct inode *inode, struct file *file);

#ifdef HAVE_PROC_OPS
static struct proc_ops fops = {
    .proc_read = fortune_read,
    .proc_write = fortune_write,
    .proc_open = fortune_open,
    .proc_release = fortune_release
};
#else
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = fortune_read,
    .write = fortune_write,
    .open = fortune_open,
    .release = fortune_release
}
#endif

void *fortune_start(struct seq_file *m, loff_t *pos);
void fortune_stop(struct seq_file *m, void *v);
void *fortune_next(struct seq_file *m, void *v, loff_t *pos);
int fortune_show(struct seq_file *m, void *v);

static struct seq_operations seq_ops = {
    .start = fortune_start,
    .stop = fortune_stop,
    .next = fortune_next,
    .show = fortune_show,
};

static char *cookie_pot = NULL;
static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;
static unsigned int read_index = 0;
static unsigned int write_index = 0;

void *fortune_start(struct seq_file *m, loff_t *pos)
{
    printk(KERN_INFO "** INFO: call fortune_start\n");

    if (*pos > 0 || write_index == 0)
    {
        *pos = 0;
        return NULL;
    }

    if (read_index > write_index)
        read_index = 0;

    return cookie_pot + read_index;
}

void fortune_stop(struct seq_file *m, void *v)
{
    printk(KERN_INFO "** INFO: call fortune_stop\n");

    if (v)
        printk(KERN_INFO "v is %pX\n", v);
    else
        printk(KERN_INFO "v is NULL\n");
}

void *fortune_next(struct seq_file *m, void *v, loff_t *pos)
{
    printk(KERN_INFO "** INFO: call fortune_next\n");

    read_index++;
    (*pos)++;

    return (*((char *)v) == '\0') ? NULL : (char *)v + 1;
}

int fortune_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "** INFO: call fortune_show\n");
    seq_printf(m, "%c", *((char *)v));
    return 0;
}

static ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "** INFO: call fortune_read\n");
    return seq_read(file, buf, count, f_pos);
}

static int fortune_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "** INFO: call fortune_open\n");
    return seq_open(file, &seq_ops);
}

static ssize_t fortune_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    int space_left = (COOKIE_BUF_SIZE - write_index) + 1;
    printk(KERN_INFO "** INFO: call fortune_write");

    if (space_left < count) {
        printk(KERN_ERR "** ERROR: no space left\n");
        return -ENOSPC;
    }

    if (copy_from_user(&cookie_pot[write_index], buf, count)) {
        printk(KERN_ERR "** ERROR: copy_from_user\n");
        return -EFAULT;
    }

    write_index += count;
    cookie_pot[write_index - 1] = '\0';
    printk(KERN_INFO "** INFO: success fortune_write");

    return count;
}

static int fortune_release(struct inode*, struct file*)
{
    printk(KERN_INFO "** INFO: call fortune_release");
    return 0;
}

static int __init fortune_init(void) {
    cookie_pot = (char *) vmalloc(COOKIE_BUF_SIZE);

    if (!cookie_pot) {
        printk(KERN_ERR "** ERROR: can't vmalloc\n");
        return -ENOMEM;
    }

    memset(cookie_pot, 0, COOKIE_BUF_SIZE);
    proc_file = proc_create("fortune", S_IRUGO | S_IWUGO, NULL, &fops);

    if (!proc_file) {
        vfree(cookie_pot);
        printk(KERN_ERR "** ERROR: can't proc_create\n");
        return -ENOMEM;
    }

    read_index = 0;
    write_index = 0;

    proc_dir = proc_mkdir("fortune_dir", NULL);
    proc_link = proc_symlink("fortune_symlink", NULL, "/proc/fortune");
    
    if (!proc_dir || !proc_link) {
        vfree(cookie_pot);
        printk(KERN_ERR "** ERROR: can't proc_mkdir or proc_symlink\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "** INFO: fortune module loaded\n");

    return 0;
}

static void __exit fortune_exit(void) {
    proc_remove(proc_file);
    proc_remove(proc_dir);
    proc_remove(proc_link);
    vfree(cookie_pot);
    printk(KERN_INFO "** INFO: fortune module unloaded\n");
}

module_init(fortune_init);
module_exit(fortune_exit);
