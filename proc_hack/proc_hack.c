/*
 *  proc_hack.c
 *
 *  add hook in proc_pid_readdir() in fs/proc/base.c
 *  The kernel should hava appropriate hook point.
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
#include <linux/proc_fs.h>
#include <linux/mm.h> /* VM_READ etc */

/* 
 * For the current (process) structure, we need
 * this to know who the current user is. 
 */
#include <linux/sched.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");


proc_pid_readdir_hook original_in_proc_pid_readdir;

int func_proc_pid_readdir (struct tgid_iter *iter) {
  int ret = 0;
  if (current->hp_node >= 0) {
    if (iter->task->hp_node != current->hp_node) {
      printk("*** %s ", iter->task->comm);
      printk(" <- %s", current->comm);
      printk("  *** skipped! ***");
      printk("\n");
      ret = 1;
    }
  }
  return ret;
}


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
  return -1;
}


/*
  Traverse Tasks, marking to-be-traced if it is one of target processes.
*/
void mark_process(void) {
  struct task_struct *task = &init_task;
  int node_index;
  do {
    if ((node_index = is_target_proc(task->pid)) >= 0) {
      task->hp_node = node_array[node_index];
      printk(KERN_INFO "*** %s [%ld] parent %s\n",
             task->comm, task->hp_node, task->parent->comm);
    } else {
      task->hp_node = -1;
    }
  } while ((task = next_task(task)) != &init_task);
}


/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{
  mark_process();

  /* Any lock ? */

  original_in_proc_pid_readdir = honeypot_hooks.in_proc_pid_readdir;
  honeypot_hooks.in_proc_pid_readdir = func_proc_pid_readdir;

  if (honeypot_hooks.in_proc_pid_readdir == func_proc_pid_readdir) {
    printk("Successfully installed\n");
  }

  return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
  if (honeypot_hooks.in_proc_pid_readdir == func_proc_pid_readdir) {
    honeypot_hooks.in_proc_pid_readdir = original_in_proc_pid_readdir;
    printk("ended process module\n");
  } else {
    printk("Something wrong!\n");
  }

  return;
}
