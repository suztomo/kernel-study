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
  printk("*** %s ", iter->task->comm);
  printk(" <- %s", current->comm);
  if (strcmp(iter->task->comm, "sshd") == 0) {
    printk("  *** skipped! ***");
    ret = 1;
  }
  printk("\n");
  return ret;
}


/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{
  struct task_struct *task = &init_task;
  do {
    printk(KERN_INFO "*** %s [%d] parent %s\n",
           task->comm, task->pid, task->parent->comm);
    task->hp_node = -1;
  } while ((task = next_task(task)) != &init_task);

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
