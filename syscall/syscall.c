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
#include <asm-cris/cacheflush.h> /* change_page_attr */

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
void **sys_call_table_local;


unsigned long **find_sys_call_table(void)
{

  unsigned long **sctable;
  unsigned long ptr;
  sctable = NULL;
  for (ptr = (unsigned long)&strstr;
       ptr < (unsigned long)&boot_cpu_data;
       ptr += sizeof(void*)) {

    unsigned long *p;
    p = (unsigned long *)ptr;
    if (p[__NR_close] == (unsigned long) sys_close) {
      sctable = (unsigned long **)p;
      return &sctable[0];
    }
  }
  return NULL;
}

/* 
 * UID we want to spy on - will be filled from the
 * command line 
 */
static int uid;
module_param(uid, int, 0644);

/* 
 * A pointer to the original system call. The reason
 * we keep this, rather than call the original function
 * (sys_open), is because somebody else might have
 * replaced the system call before us. Note that this
 * is not 100% safe, because if another module
 * replaced sys_open before us, then when we're inserted
 * we'll call the function in that module - and it
 * might be removed before we are.
 *
 * Another reason for this is that we can't get sys_open.
 * It's a static variable, so it is not exported. 
 */
asmlinkage int (*original_call) (const char *, int, int);

/* 
 * The function we'll replace sys_open (the function
 * called when you call the open system call) with. To
 * find the exact prototype, with the number and type
 * of arguments, we find the original function first
 * (it's at fs/open.c).
 *
 * In theory, this means that we're tied to the
 * current version of the kernel. In practice, the
 * system calls almost never change (it would wreck havoc
 * and require programs to be recompiled, since the system
 * calls are the interface between the kernel and the
 * processes).
 */
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
		printk("Opened file by %d: ", uid);
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
  pgprot_t prot;

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
  sys_call_table_local = (void **)find_sys_call_table();
  if (sys_call_table_local == NULL) {
    return 1;
  }

  pg = virt_to_page(sys_call_table_local);
  prot.pgprot = VM_READ | VM_WRITE | VM_EXEC; /* R-W-X */
  change_page_attr(pg, 1, prot);

  printk(KERN_INFO "sys_call_table is %p\n", sys_call_table_local);

  printk(KERN_INFO "assumed close is %p\n", sys_call_table_local[__NR_close]);
  printk(KERN_INFO "real    close is %p\n", sys_close);



	/* 
	 * Keep a pointer to the original function in
	 * original_call, and then replace the system call
	 * in the system call table with our_sys_open 
	 */
    original_call = sys_call_table_local[__NR_open];
    printk(KERN_INFO "old open is %p\n", original_call);
    printk(KERN_INFO "new open is %p\n", our_sys_open);

    sys_call_table_local[__NR_open] = our_sys_open;


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
  printk(KERN_INFO "sys_call_table: %p\n", sys_call_table_local);
  printk(KERN_INFO "current open  : %p\n", sys_call_table_local[__NR_open]);
  printk(KERN_INFO "our open      : %p\n", our_sys_open);

  if (sys_call_table_local[__NR_open] != our_sys_open) {
    printk(KERN_ALERT "Somebody else also played with the ");
    printk(KERN_ALERT "open system call\n");
    printk(KERN_ALERT "The system may be left in ");
    printk(KERN_ALERT "an unstable state.\n");
  }
  sys_call_table_local[__NR_open] = original_call;

  printk(KERN_INFO "Ended spying on UID: %d\n", uid);
}