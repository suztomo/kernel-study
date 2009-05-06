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

#include <linux/highmem.h>
#include <linux/mm.h> /* VM_READ etc */
#include <asm-x86/cacheflush.h> /* change_page_attr */

#include <linux/vmalloc.h>
#include <linux/kallsyms.h>

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
//extern void *sys_call_table[];
void **sys_call_table;
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

  /* 
   * Check if this is the user we're spying on 
   */
  if (uid == current->uid) {
    /* 
     * Report the file, if relevant 
     */
    printk(KERN_INFO "Opened file by %d: ", uid);
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
  return original_call(filename, flags, mode);
}

/* 
 * Initialize the module - replace the system call 
 */
int init_module()
{
  struct page *pg;
  unsigned long target, init_target;
  unsigned long open_addr = (unsigned long)&(sys_call_table[__NR_open]);

  sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");

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



  if (sys_call_table == NULL) {
    return 1;
  }
  /*

  prot.pgprot = VM_READ | VM_WRITE | VM_EXEC;
  change_page_attr(pg, 1, prot); // obsolete
  */
  pg = virt_to_page(open_addr);
  init_target = (unsigned long)vmap(&pg, 1, VM_MAP, PAGE_KERNEL);
  target = init_target + (open_addr & 0xFFF);

  printk(KERN_INFO "target is %p\n", (void **)target);


  /* 
   * Keep a pointer to the original function in
   * original_call, and then replace the system call
   * in the system call table with our_sys_open 
   */
  original_call = sys_call_table[__NR_open];
  printk(KERN_INFO "sys_call_table %p", sys_call_table);
  printk(KERN_INFO "original sys_open is %p\n", original_call);

  printk(KERN_INFO "sys_call_table[__NR_open] : %p %p\n", original_call, *(void **)target);

  *(void **)target = our_sys_open;

  printk(KERN_INFO "sys_call_table[__NR_open] : %p %p\n", sys_call_table[__NR_open],
         *(void **)target);


  printk(KERN_INFO "now, sys_open is %p\n", sys_call_table[__NR_open]);
  printk(KERN_INFO "our sys_open is %p\n", our_sys_open);

  //   *(void **)target = original_call;
  //  sys_call_table_local[__NR_open] = our_sys_open;

  vunmap((void *)init_target);


  printk(KERN_INFO "after unmap, sys_open is %p\n", sys_call_table[__NR_open]);

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
  void **open_addr;
  void **target, **init_target;
  struct page *pg;
  open_addr = &(sys_call_table[__NR_open]);
  pg = virt_to_page(open_addr);
  init_target = vmap(&pg, 1, VM_MAP, PAGE_KERNEL);
  target = (void **)((unsigned long)init_target + ((unsigned long)open_addr & 0xFFF));

  //  * Return the system call back to normal
  printk(KERN_INFO "sys_call_table: %p\n", sys_call_table);
  printk(KERN_INFO "current open  : %p\n", sys_call_table[__NR_open]);
  printk(KERN_INFO "target ope n  : %p\n", *target);
  printk(KERN_INFO "our open      : %p\n", our_sys_open);
  printk(KERN_INFO "our original  : %p\n", original_call);


  if (*target != our_sys_open) {
    printk(KERN_ALERT "Somebody else also played with the ");
    printk(KERN_ALERT "open system call\n");
    printk(KERN_ALERT "The system may be left in ");
    printk(KERN_ALERT "an unstable state.\n");
  } else {
    *target = original_call;
  }

  vunmap(init_target);
  printk(KERN_INFO "Ended spying on UID: %d\n", uid);
}
