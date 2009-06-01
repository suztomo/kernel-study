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
#include <asm-x86/cacheflush.h> /* change_page_attr */

/* 
 * For the current (process) structure, we need
 * this to know who the current user is. 
 */
#include <linux/sched.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");


/* 
 * UID of the initial processes owner.


static int initial_uid;
module_param(initial_uid, int, 0644);
 */

/*
  Gets a mapping of (process id -> node id)
 */
int pid_array[10];
int pid_array_count;
module_param_array(pid_array, int, &pid_array_count, 0000);

int node_array[10];
int node_array_count;
module_param_array(node_array, int, &node_array_count, 0000);

/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{
  int i;
  struct task_struct *task = &init_task;

  if (!pid_array_count) {
    printk(KERN_INFO "specify node_array and pid_array\n");
    return -1;
  }

  printk(KERN_INFO "pid_array %d\n", pid_array_count);


  if (pid_array_count != node_array_count) {
    printk(KERN_INFO "Array size mismatch: pid_array and node_array\n");
    return -1;
  }

  for (i=0; i<pid_array_count; ++i) {
    printk(KERN_INFO "*** %d : %d\n", pid_array[i], node_array[i]);
  }

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
