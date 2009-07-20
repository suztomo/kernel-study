/*
 *  syscall.c
 *
 *  System call "stealing" sample.
 */

/* 
 * The necessary header files 
 */

/*
 * Standard in kernel modules 
 */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/syscalls.h> /* sys_close */
#include <linux/module.h>	/* Specifically, a module, */
#include <linux/moduleparam.h>	/* which will have params */
#include <linux/unistd.h>	/* The list of system calls */

#include <linux/mm.h> /* VM_READ etc */

/* 
 * For the current (process) structure, we need
 * this to know who the current user is. 
 */
#include <linux/sched.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");


/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{
  struct task_struct *task = &init_task;

  do {
    printk(KERN_INFO "*** %s [%d] parent %s\n",
           task->comm, task->pid, task->parent->comm);
    //      task->trace_nid != ;
    /*
      if (task->trace_nid != 123) {
      printk(KERN_INFO "Fucking trace_nid %d\n", task->trace_nid);
      }
    */
  } while ((task = next_task(task)) != &init_task);
  return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
  printk("ended process module\n");
  return;
}
