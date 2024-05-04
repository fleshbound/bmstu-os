#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

#define myvfs_MAGIC_NUMBER 0xB16B00B5

static struct myvfs_inode
{
    int i_mode;
    unsigned long i_ino;
} myvfs_inode;

static void myvfs_put_super(struct super_block *sb)
{
    printk(KERN_INFO "+ myvfs: myfs_put_super\n");
}

static struct super_operations const myvfs_super_ops = {
    .put_super = myvfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

static struct inode *myvfs_make_inode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);
    
    if (ret)
    {
        inode_init_owner(&nop_mnt_idmap, ret, NULL, mode);

        ret->i_ino = 727;
        printk(KERN_INFO "+ myvfs: root inode i_ino = %lu", ret->i_ino);

        ret->i_mode = mode;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = &myvfs_inode;
    }

    return ret;
}

static int myvfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    struct inode *inode;

    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = myvfs_MAGIC_NUMBER;
    sb->s_op = &myvfs_super_ops;

    inode = myvfs_make_inode(sb, S_IFDIR | 0755);
    
    if (!inode)
    {
        printk(KERN_ERR "+ myvfs: can't new_inode root\n");
        return -ENOMEM;
    }
    
    inode->i_op = &simple_dir_inode_operations;
    inode->i_fop = &simple_dir_operations;

    sb->s_root = d_make_root(inode);

    if (!sb->s_root)
    {
        printk(KERN_ERR "+ myvfs: can't d_make_root\n");
        iput(inode);
        return -ENOMEM;
    }

    return 0;
}

static struct dentry *myvfs_mount(struct file_system_type *type, int flags, const char *dev, void *data)
{
    struct dentry *const entry = mount_nodev(type, flags, data, myvfs_fill_sb);

    if (IS_ERR(entry))
        printk(KERN_ERR "+ myvfs mounting failed\n");
    else
        printk(KERN_INFO "+ myvfs mounted\n");

    return entry;
}

static void myvfs_kill_sb(struct super_block *sb)
{
    printk(KERN_INFO "+ myvfs: kill super block\n");
    kill_anon_super(sb);
}

static struct file_system_type myvfs_type = {
    .owner = THIS_MODULE,
    .name = "myvfs",
    .mount = myvfs_mount,
    .kill_sb = myvfs_kill_sb,
    .fs_flags = FS_USERNS_MOUNT,
};

static int __init myvfs_init(void)
{
    int ret = register_filesystem(&myvfs_type);

    if (ret)
    {
        printk(KERN_ERR "+ myvfs: can't register_filesystem\n");
        return ret;
    }

    printk(KERN_INFO "+ myvfs: module loaded\n");

    return 0;
}

static void __exit myvfs_exit(void)
{
    int ret = unregister_filesystem(&myvfs_type);

    if (ret)
        printk(KERN_ERR "+ myvfs: can't unregister_filesystem\n");

    printk(KERN_INFO "+ myvfs: module unloaded\n");
}

module_init(myvfs_init);
module_exit(myvfs_exit);
