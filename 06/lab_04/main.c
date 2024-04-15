#include <linux/init.h>  
#include <linux/init_task.h>  
#include <linux/kernel.h>     
#include <linux/module.h>     
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/path.h>

MODULE_LICENSE("GPL");

static int __init mod_init(void) {
    struct task_struct *task = &init_task;
    do
    {
        printk(KERN_INFO "- taskp - %llu, comm - %s, pid - %d, parent comm - %s, ppid - %d, state - %d, on_cpu - %d, flags - %x, prio - %d, policy - %d, migration flags - %hu, exit_state - %d, exit_code - %d, in_execve - %x, utime - %lu, thread_pid - %d, root - %s\n", 
            task,
            task->comm,
            task->pid,
            task->parent->comm,
            task->parent->pid,
            task->__state,
            task->on_cpu,
            task->flags,
            task->prio,
            task->policy,
            task->migration_flags,
            task->exit_state,
            task->exit_code,
            task->in_execve,
            task->utime,
            task->fs->root.dentry->d_name.name
        );
    } while ((task = next_task(task)) != &init_task);

    printk(KERN_INFO "- CURRENT taskp - %llu, comm - %s, pid - %d, parent comm - %s, ppid - %d, state - %d, on_cpu - %d, flags - %x, prio - %d, policy - %d, migration flags - %hu, exit_state - %d, exit_code - %d, in_execve - %x, utime - %lu, thread_pid - %d, root - %s\n", 
        current,
        current->comm,
        current->pid,
        current->parent->comm,
        current->parent->pid,
        current->__state,
        current->on_cpu,
        current->flags,
        current->prio,
        current->policy,
        current->migration_flags,
        current->exit_state,
        current->exit_code,
        current->in_execve,
        current->utime,
        current->thread_pid,
        current->fs->root.dentry->d_name.name
    );

    return 0;
}

static void __exit mod_exit(void) {}

module_init(mod_init);
module_exit(mod_exit);
