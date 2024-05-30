#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

#define myvfs_MAGIC_NUMBER 0xB16B00B5
#define MAX_CACHE_SIZE PAGE_SIZE
#define SLAB_NAME "myvfs_cache"

static struct kmem_cache *cache = NULL;
static void **cache_mem = NULL;
static int cached_count = 1;
module_param(cached_count, int, 0);

struct myvfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

static void myvfs_put_super(struct super_block *sb);
static int myvfs_statfs(struct dentry *denty, struct kstatfs *buf);
int myvfs_delete_inode(struct inode *inode);

static struct super_operations const myvfs_super_operations = {
    .put_super = myvfs_put_super,
    .statfs = myvfs_statfs,
    .drop_inode = myvfs_delete_inode,
};

static int myvfs_fill_super(struct super_block *sb, void *data, int silent);
static struct dentry *myvfs_mount(struct file_system_type *type, int flags, const char *dev, void *data);
static void myvfs_kill_super(struct super_block *sb);

static struct file_system_type myvfs_type = {
    .owner = THIS_MODULE,
    .name = "myvfs",
    .mount = myvfs_mount,
    .kill_sb = myvfs_kill_super,
    .fs_flags = FS_USERNS_MOUNT,
};

static void myvfs_put_super(struct super_block *sb)
{
    printk(KERN_INFO "+ myvfs: myfs_put_super\n");
}

static int myvfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
    printk(KERN_INFO "+ myvfs: call myvfs_statfs\n");
    return simple_statfs(dentry, buf);
}

int myvfs_delete_inode(struct inode *inode)
{
    printk(KERN_INFO "+ myvfs: call myvfs_delete_inode\n");
    return generic_delete_inode(inode);
}

static int myvfs_fill_super(struct super_block *sb, void *data, int silent)
{
    printk(KERN_INFO "+ myvfs: call myvfs_fill_super\n");
    struct inode *inode;

    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = myvfs_MAGIC_NUMBER;
    sb->s_op = &myvfs_super_operations;
    sb->s_time_gran = 1;

    inode = new_inode(sb);

    if (!inode)
    {
        printk(KERN_ERR "+ myvfs: ERR: new_inode\n");
        return -ENOMEM;
    }

    inode->i_ino = 1;
    inode->i_mode = S_IFDIR | 0755;
    inode->i_ctime = inode->i_mtime = inode->i_atime = current_time(inode);
    inode->i_op = &simple_dir_inode_operations;
    inode->i_fop = &simple_dir_operations;
    set_nlink(inode, 2);
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
    {
        printk(KERN_ERR "+ myvfs: ERR: d_make_root\n");
        iput(inode);
        return -ENOMEM;
    }
    return 0;
}

static struct dentry *myvfs_mount(struct file_system_type *type, int flags, const char *dev, void *data)
{
    struct dentry *const entry = mount_nodev(type, flags, data, myvfs_fill_super);

    if (IS_ERR(entry))
        printk(KERN_ERR "+ myvfs: mounting failed\n");
    else
        printk(KERN_INFO "+ myvfs: mounted\n");

    return entry;
}

static void myvfs_kill_super(struct super_block *sb)
{
    printk(KERN_INFO "+ myvfs: call myvfs_kill_super\n");
    kill_anon_super(sb);
}

void f_init(void *p)
{
    *(int *)p = (int)p;
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

    for (int i = 0; i < cached_count; i++)
        if (NULL == (cache_mem[i] = kmem_cache_alloc(cache, GFP_KERNEL)))
        {
            printk(KERN_ERR "+ myvfs: can't kmem_cache_alloc\n");
            kmem_cache_free(cache, *cache_mem);
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

    printk(KERN_INFO "+ myvfs: module unloaded-------------END\n");
}

module_init(myvfs_init);
module_exit(myvfs_exit);

MODULE_AUTHOR("Avdeykina Valeria");

