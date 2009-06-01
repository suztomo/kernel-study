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
  Gets a mapping of (process id -> node id)
  Fill unused entries with -1.
 */
#define PID_ARRAY_MAX 16
int pid_array[PID_ARRAY_MAX];
int pid_array_count;
module_param_array(pid_array, int, &pid_array_count, 0000);

int node_array[PID_ARRAY_MAX];
int node_array_count;
module_param_array(node_array, int, &node_array_count, 0000);


int is_target_proc(pid_t candidate_pid)
{
  int i;
  int p_i = (int)candidate_pid;
  for (i=0; i<pid_array_count; ++i) {
    if (pid_array[i] < 0) {
      break;
    }
    if (pid_array[i] == p_i) {
      return i;
    }
  }
  return 0;
}

int check_args(void) {
  int i;
  if (!pid_array_count) {
    printk(KERN_INFO "specify node_array and pid_array\n");
    return -1;
  }

  if (pid_array_count != node_array_count) {
    printk(KERN_INFO "Array size mismatch: pid_array and node_array\n");
    return -1;
  }

  printk(KERN_INFO "pid_array %d\n", pid_array_count);


  for (i=pid_array_count; i<PID_ARRAY_MAX; ++i) {
    node_array[i] = pid_array[i] = -1;
  }

  for (i=0; i<PID_ARRAY_MAX; ++i) {
    printk(KERN_INFO "*** %d : %d\n", pid_array[i], node_array[i]);
  }
  return 0;
}

/*
  Traverse Tasks, marking to-be-traced if it is one of target processes.
*/
void mark_process(void) {
  struct task_struct *task = &init_task;
  int node_id;
  do {
    if ((node_id = is_target_proc(task->pid)) != 0) {
      task->trace_nid = node_id;
      printk(KERN_INFO "*** %s [%d] parent %s\n",
             task->comm, task->trace_nid, task->parent->comm);
    }
  } while ((task = next_task(task)) != &init_task);
}

int add_hook_sysopen(void) {
  return 0;
}

/* 
 * Initialize the module - replace the system call 
 */


int init_module()
{
  /*
    Checks arguments passed with insmod
   */
  if (check_args() < 0) {
    return -1;
  }

  mark_process();

  if (add_hook_sysopen() != 0) {
    printk(KERN_INFO "add_hook_sysopen failed.\n");
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
