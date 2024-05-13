#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");

#define COOKIE_BUF_SIZE PAGE_SIZE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
static int fortune_open(struct inode*, struct file*);
static int fortune_release(struct inode*, struct file*);

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

static char *cookie_pot;
static struct proc_dir_entry *proc_file, *proc_dir, *proc_link;
static unsigned int read_index;
static unsigned int write_index;
static char tmp[256];

static ssize_t fortune_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_INFO "** INFO: call fortune_read");
    
    if (*f_pos || write_index == 0) {
        *f_pos = 0;
        return 0;
    }

    if (read_index >= write_index) {
        read_index = 0;
    }

    int len = 0;

    len = sprintf(tmp, "%s\n", &cookie_pot[read_index]);
    
    if (copy_to_user(buf, tmp, len)) {
        printk(KERN_ERR "** ERROR: copy_to_user\n");
        return -EFAULT;
    }

    buf += len;
    read_index += len;

    *f_pos += len;
    printk(KERN_INFO "** INFO: success fortune_read");

    return len;
}

static ssize_t fortune_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_INFO "** INFO: call fortune_write");

    int space_left = (COOKIE_BUF_SIZE - write_index) + 1;

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

static int fortune_open(struct inode*, struct file*) {
    printk(KERN_INFO "** INFO: call fortune_open");
    return 0;
}

static int fortune_release(struct inode*, struct file*) {
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

    proc_dir = proc_mkdir("fortune_dir", NULL);
    
    if (!proc_dir) {
        vfree(cookie_pot);
        printk(KERN_ERR "** ERROR: can't proc_mkdir\n");
        return -ENOMEM;
    }
    
    proc_link = proc_symlink("fortune_symlink", NULL, "/proc/fortune");

    if (!proc_link) {
        vfree(cookie_pot);
        printk(KERN_ERR "** ERROR: can't proc_symlink\n");
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
