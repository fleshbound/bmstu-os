#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

#define myvfs_MAGIC_NUMBER 0xB16B00B5
#define MAX_CACHE_SIZE 128
#define SLAB_NAME "myvfs_cache"

static struct kmem_cache *cache = NULL;
static void **cache_mem = NULL;
static int cached_count = 0;

struct myvfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

static void myvfs_put_super(struct super_block *sb)
{
    printk(KERN_INFO "+ myvfs: myfs_put_super\n");
}

static struct super_operations const myvfs_super_ops = {
    .put_super = myvfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

static struct myvfs_inode *myvfs_cache_get_inode(void)
{
    printk(KERN_INFO "+ myvfs: call myvfs_cache_get_inode\n");

    if (cached_count == MAX_CACHE_SIZE)
    {
        printk(KERN_ERR "+ myvfs: reached MAX_CACHE_SIZE\n");
        return NULL;
    }

    void *ret = cache_mem[cached_count++] = kmem_cache_alloc(cache, GFP_KERNEL);

    if (!ret)
    {
        printk(KERN_ERR "+ myvfs: can't kmem_cache_alloc\n");
        kmem_cache_free(cache, *cache_mem);
        kmem_cache_destroy(cache);
        kfree(cache_mem);
    }

    return ret;
}

static struct inode *myvfs_make_inode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);
    struct myvfs_inode *cache_inode = NULL;
    
    if (ret)
    {
        inode_init_owner(&nop_mnt_idmap, ret, NULL, mode);

        ret->i_ino = 1;
        printk(KERN_INFO "+ myvfs: root inode i_ino = %lu", ret->i_ino);

        ret->i_mode = mode;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);

        if (NULL == (cache_inode = myvfs_cache_get_inode()))
        {
            iput(ret);
        }
        else
        {
            cache_inode->i_mode = ret->i_mode;
            cache_inode->i_ino = ret->i_ino;
            ret->i_private = cache_inode;
        }
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

void f_init(void *p)
{
    *(int *)p = (int) p;
}

static int __init myvfs_init(void)
{
    int ret = register_filesystem(&myvfs_type);

    if (ret)
    {
        printk(KERN_ERR "+ myvfs: can't register_filesystem\n");
        return ret;
    }

    if (NULL == (cache_mem = kmalloc(sizeof(struct myvfs_inode *) * MAX_CACHE_SIZE, GFP_KERNEL)))
    {
        printk(KERN_ERR "+ myvfs: can't kmalloc cache\n");
        return -ENOMEM;
    }

    if (NULL == (cache = kmem_cache_create(SLAB_NAME, sizeof(struct myvfs_inode), 0, SLAB_HWCACHE_ALIGN, f_init)))
    {
        printk(KERN_ERR "+ myvfs: can't kmem_cache_create\n");
        kmem_cache_destroy(cache);
        kfree(cache_mem);
        return -ENOMEM;
    }

    printk(KERN_INFO "+ myvfs: alloc %d objects into slab: %s\n", cached_count, SLAB_NAME);
    printk(KERN_INFO "+ myvfs: object size %ld bytes, full size %ld bytes\n", sizeof(struct myvfs_inode), sizeof(struct myvfs_inode *) * MAX_CACHE_SIZE);
    printk(KERN_INFO "+ myvfs: module loaded\n");

    return 0;
}

static void __exit myvfs_exit(void)
{
    int ret = unregister_filesystem(&myvfs_type);

    if (ret)
        printk(KERN_ERR "+ myvfs: can't unregister_filesystem\n");

    for (size_t i = 0; i < cached_count; i++)
        kmem_cache_free(cache, cache_mem[i]);

    kmem_cache_destroy(cache);
    kfree(cache_mem);

    printk(KERN_INFO "+ myvfs: module unloaded\n");
}

module_init(myvfs_init);
module_exit(myvfs_exit);
