#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

#define MYVFS_MAGIC_NUMBER 0xB16B00B5
#define MAX_CACHE_SIZE 128
#define SLAB_NAME "myvfs_cache1"

static struct kmem_cache *cache = NULL;
static void **cache_mem = NULL;
static int cached_count = 1;

struct myvfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

static struct tree_descr files[] = {
	    { NULL, NULL, 0 },
};

static int myvfs_fill_super(struct super_block *sb, void *data, int silent)
{
    return simple_fill_super(sb, MYVFS_MAGIC_NUMBER, files);
}

static struct dentry *myvfs_mount(struct file_system_type *type, int flags, const char *dev, void *data)
{
    struct dentry *const entry = mount_nodev(type, flags, data, myvfs_fill_super);

    if (IS_ERR(entry))
        printk(KERN_ERR "+ ERR: myvfs mounting failed\n");
    else
        printk(KERN_INFO "+ INFO: myvfs mounted\n");

    return entry;
}

void my_kill_super(struct super_block *sb)
{
    return kill_litter_super(sb);
}

static struct file_system_type myvfs_type = {
    .owner = THIS_MODULE,
    .name = "myvfs",
    .mount = myvfs_mount,
    .kill_sb = my_kill_super,
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
        printk(KERN_ERR "+ ERR: myvfs: can't register_filesystem\n");
        return ret;
    }

    if (NULL == (cache_mem = kmalloc(sizeof(struct myvfs_inode *) * MAX_CACHE_SIZE, GFP_KERNEL)))
    {
        printk(KERN_ERR "+ ERR: myvfs: can't kmalloc cache\n");
        return -ENOMEM;
    }

    if (NULL == (cache = kmem_cache_create(SLAB_NAME, sizeof(struct myvfs_inode), 0, SLAB_HWCACHE_ALIGN, f_init)))
    {
        printk(KERN_ERR "+ ERR: myvfs: can't kmem_cache_create\n");
        kmem_cache_destroy(cache);
        kfree(cache_mem);
        return -ENOMEM;
    }

    for (int i = 0; i < cached_count; i++)
        if (NULL == (cache_mem[i] = kmem_cache_alloc(cache, GFP_KERNEL)))
        {
            printk(KERN_ERR "+ ERR: myvfs: can't kmem_cache_alloc\n");

            for (size_t j = 0; j < i; j++)
                kmem_cache_free(cache, cache_mem[j]);
            
            kmem_cache_destroy(cache);
            kfree(cache_mem);

            return -ENOMEM;
        }

    printk(KERN_INFO "+ INFO: myvfs: alloc %d objects into slab: %s\n", cached_count, SLAB_NAME);
    printk(KERN_INFO "+ INFO: myvfs: object size %ld bytes, MAX = %d, full size %ld bytes\n", sizeof(struct myvfs_inode), MAX_CACHE_SIZE, sizeof(struct myvfs_inode) * MAX_CACHE_SIZE);
    printk(KERN_INFO "+ INFO: myvfs: module loaded\n");

    return 0;
}

static void __exit myvfs_exit(void)
{
    for (size_t i = 0; i < cached_count; i++)
        kmem_cache_free(cache, cache_mem[i]);

    kmem_cache_destroy(cache);
    kfree(cache_mem);
    
    int ret = unregister_filesystem(&myvfs_type);

    if (ret)
        printk(KERN_ERR "+ ERR: myvfs: can't unregister_filesystem\n");

    printk(KERN_INFO "+ INFO: myvfs: module unloaded-------------END\n");
}

module_init(myvfs_init);
module_exit(myvfs_exit);
