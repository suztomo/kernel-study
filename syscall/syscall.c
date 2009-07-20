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
 * The system call table (a table of functions). We
 * just define this as external, and the kernel will
 * fill it up for us when we are insmod'ed
 *
 * sys_call_table_local is no longer exported in 2.6.x kernels.
 * If you really want to try this DANGEROUS module you will
 * have to apply the supplied patch against your current kernel
 * and recompile it.
 */
extern void *sys_call_table[];
//void **sys_call_table = (void **)0xc0385880;

//void *sys_call_table_local;
//void **sys_call_table_local[];

/* 
 * UID we want to spy on - will be filled from the
 * command line 
 */
static int uid;
module_param(uid, int, 0644);

asmlinkage int (*original_call) (const char *, int, int);

asmlinkage int our_sys_open(const char *filename, int flags, int mode)
{
  int i = 0;
  char ch;
  int ret;

  /* 
   * Check if this is the user we're spying on 
   */
  if (uid == current->cred->uid) {
    /* 
     * Report the file, if relevant 
     */
    printk("1: Opened file by %d: ", uid);
    do {
      get_user(ch, filename + i);
      i++;
      printk("%c", ch);
    } while (ch);
    printk("\n");
  }

  /* 
   * Call the original sys_open - otherwise, we lose
   * the ability to open files 
   */
  ret = original_call(filename, flags, mode);
  return ret;
}

/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{

  /*
    struct page *pg;
    pgprot_t prot;
  */
  /* 
   * Warning - too late for it now, but maybe for
   * next time... 
   */
  printk(KERN_ALERT "I'm dangerous. I hope you did a ");
  printk(KERN_ALERT "sync before you insmod'ed me.\n");
  printk(KERN_ALERT "My counterpart, cleanup_module(), is even");
  printk(KERN_ALERT "more dangerous. If\n");
  printk(KERN_ALERT "you value your file system, it will ");
  printk(KERN_ALERT "be \"sync; rmmod\" \n");
  printk(KERN_ALERT "when you remove this module.\n");



  //    sys_call_table_local = (void **)0xc0385880;
  //  sys_call_table = (void **)find_sys_call_table();
  if (sys_call_table == NULL) {
    return 1;
  }
  /*
    pg = virt_to_page(sys_call_table_local);
    prot.pgprot = VM_READ | VM_WRITE | VM_EXEC;
    change_page_attr(pg, 1, prot); // obsolete
  */
  printk(KERN_INFO "sys_call_table is %p\n", sys_call_table);

  printk(KERN_INFO "assumed open is %p\n", sys_call_table[__NR_open]);
  //  printk(KERN_INFO "real    open is %p\n", sys_open);



  /* 
   * Keep a pointer to the original function in
   * original_call, and then replace the system call
   * in the system call table with our_sys_open 
   */
  original_call = sys_call_table[__NR_open];

  /*
  printk(KERN_INFO "old open is %p\n", original_call);
  printk(KERN_INFO "going to write at %p", &(sys_call_table[__NR_open]));
  printk(KERN_INFO "new open is %p\n", our_sys_open);
  printk(KERN_INFO "new open is %p\n", our_sys_open);
  */

  sys_call_table[__NR_open] = our_sys_open;
  //    sys_call_table[__NR_open] = original_call;


  /* 
   * To get the address of the function for system
   * call foo, go to sys_call_table[__NR_foo]. 
   */

  printk(KERN_INFO "Spying on UID:%d\n", uid);

  return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{

  //  * Return the system call back to normal
  printk(KERN_INFO "sys_call_table: %p\n", sys_call_table);
  printk(KERN_INFO "current open  : %p\n", sys_call_table[__NR_open]);
  printk(KERN_INFO "our open      : %p\n", our_sys_open);

  if (sys_call_table[__NR_open] != our_sys_open) {
    printk(KERN_ALERT "Somebody else also played with the ");
    printk(KERN_ALERT "open system call\n");
    printk(KERN_ALERT "The system may be left in ");
    printk(KERN_ALERT "an unstable state.\n");
  } else {
    sys_call_table[__NR_open] = original_call;
  }

  printk(KERN_INFO "Ended spying on UID: %d\n", uid);
}
