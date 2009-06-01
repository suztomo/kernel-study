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

#define HOMEDIR_PREFIX "/home/"
#define BACKUP_LEN 8

/*
  The system call table.
  The kernel should be modified as EXPORT_SYMBOL(sys_call_table)
 */
extern void *sys_call_table[];


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
  int node_index;
  do {
    if ((node_index = is_target_proc(task->pid)) >= 0) {
      task->trace_nid = node_array[node_index];
      printk(KERN_INFO "*** %s [%d] parent %s\n",
             task->comm, task->trace_nid, task->parent->comm);
    } else {
      task->trace_nid = -1;
    }
  } while ((task = next_task(task)) != &init_task);
}


asmlinkage int (*original_call) (const char *, int, int);
//asmlinkage int (*original_syschdir) (const char*);

#define REPLACE_SYSCALL(call) \
  asmlinkage int (*original_sys##call) (const char *);
REPLACE_SYSCALL(chdir)

asmlinkage int sys_chdir_wrapper(/* const */ char *filename)
{
  int i;
  int ret;
  char ch;
  char tmp_buf[32];
  char filename_prefix[12];

  if (current->trace_nid <= 0) {
    /*
      When the current process is not our target
    */
    return original_syschdir(filename);
  }

  i = 0;
  printk("chdir to :");
  do {
    get_user(ch, filename + i);
    i++;
    printk("%c", ch);
  } while (ch && i < 200);
  printk("\n");





  for (i=0; i<12; i++) {
    get_user(filename_prefix[i], filename+i);
  }


  if (strncmp(filename_prefix, HOMEDIR_PREFIX, strlen(HOMEDIR_PREFIX)) != 0) {
    printk("Not home dir");
    return original_syschdir(filename);
  }

  /*
    Going to replace parameter
   */
  for (i=0; i<BACKUP_LEN; ++i) {
    /* backup */
    get_user(current->trace_buf[i], filename - 8 + i);
  }

  snprintf(tmp_buf, 12, "/j/%05d", current->trace_nid);
  for (i=0; i<BACKUP_LEN; ++i) {
    put_user(tmp_buf[i], filename - 8 + i);
  }
  printk("new dirname %s\n", filename -8 );

  /* 
   * Call the original sys_open - otherwise, we lose
   * the ability to open files 
   */
  ret = original_syschdir(filename - 8);

  /* 
   * Report the file, if relevant 
   */
  for (i=0; i<BACKUP_LEN; ++i) {
    //    printk("%c", current->trace_buf[i]);
    put_user(current->trace_buf[i], filename - 8 + i);
  }
  return ret;
}

asmlinkage int sys_open_wrapper(/* const */ char *filename, int flags, int mode)
{
  int i = 0;
  char ch;
  int ret;
  char filename_m8_bkup[12];
  char tmp_buf[32];
  char filename_prefix[12];
  /*
  unsigned long long filename_m8_bkup;
  unsigned long long *filename_m8_p = (long long *)(filename - 8);
  */
  /* 
   * Check if this is the user we're spying on 
   */

  if (current->trace_nid <= 0) {
    /*
      When the current process is not our target
    */
    return original_call(filename, flags, mode);
  }

  /* 
   * When one of our target processes
   */
  printk("1: Opened file by %d on %d %s: ", current->pid, current->trace_nid, current->comm);

  i = 0;
  do {
    get_user(ch, filename + i);
    i++;
    printk("%c", ch);
  } while (ch && i < 200);
  printk("\n");

  for (i=0; i<12; i++) {
    get_user(filename_prefix[i], filename+i);
  }


  if (strncmp(filename_prefix, HOMEDIR_PREFIX, strlen(HOMEDIR_PREFIX)) != 0) {
    printk("Not home dir");
    return original_call(filename, flags, mode);
  }

  /*
    Going to replace parameter
   */
  for (i=0; i<BACKUP_LEN; ++i) {
    /* backup */
    get_user(current->trace_buf[i], filename - 8 + i);
  }

  snprintf(tmp_buf, 12, "/j/%05d", current->trace_nid);
  for (i=0; i<BACKUP_LEN; ++i) {
    put_user(tmp_buf[i], filename - 8 + i);
  }
  printk("new filename %s", filename -8 );

  /* 
   * Call the original sys_open - otherwise, we lose
   * the ability to open files 
   */
  ret = original_call(filename - 8, flags, mode);


  /* 
   * Report the file, if relevant 
   */
  printk(KERN_INFO "2: Opened file by %d on %d %s: ", current->pid, current->trace_nid, current->comm);
  i = 0;
  do {
    get_user(ch, filename + i);
    i++;
    printk("%c", ch);
  } while (ch);
  printk("\n");

  for (i=0; i<BACKUP_LEN; ++i) {
    //    printk("%c", current->trace_buf[i]);
    put_user(current->trace_buf[i], filename - 8 + i);
  }
  printk("\n");
  return ret;
}



int add_hook_sysopen(void) {
  /* 
   * Keep a pointer to the original function in
   * original_call, and then replace the system call
   * in the system call table with our_sys_open 
   */
  original_call = sys_call_table[__NR_open];


  printk(KERN_INFO "old open is %p\n", original_call);
  printk(KERN_INFO "going to write at %p", &(sys_call_table[__NR_open]));
  printk(KERN_INFO "new open is %p\n", sys_open_wrapper);

  sys_call_table[__NR_open] = sys_open_wrapper;

  if (sys_call_table[__NR_open] != sys_open_wrapper) {
    return -1;
  } else {
    printk(KERN_INFO "open replaced successfully.\n");
  }

  return 0;
}



int add_hook_syschdir(void) {
  original_syschdir = sys_call_table[__NR_chdir];
  sys_call_table[__NR_chdir] = sys_chdir_wrapper;
  if (sys_call_table[__NR_chdir] != sys_chdir_wrapper) {
    return -1;
  } else {
    printk(KERN_INFO "chdir replaced successfully.\n");
  }
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
    return -1;
  }
  if (add_hook_syschdir() != 0) {
    printk(KERN_INFO "add_hook_syschdir failed.\n");
    return -1;
  }


  return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
  if (sys_call_table[__NR_open] != sys_open_wrapper) {
    printk(KERN_ALERT "Somebody else also played with the ");
    printk(KERN_ALERT "open system call\n");
    printk(KERN_ALERT "The system may be left in ");
    printk(KERN_ALERT "an unstable state.\n");
  } else {
    sys_call_table[__NR_open] = original_call;
    printk(KERN_INFO "replaced open as usual.\n");
  }

  if (sys_call_table[__NR_chdir] != sys_chdir_wrapper) {
    printk(KERN_ALERT "Somebody else also played with the ");
    printk(KERN_ALERT "chdir system call\n");
    printk(KERN_ALERT "The system may be left in ");
    printk(KERN_ALERT "an unstable state.\n");
  } else {
    sys_call_table[__NR_chdir] = original_syschdir;
    printk(KERN_INFO "replaced chdir as usual.\n");
  }
  printk("ended open_hack module\n");
  return;
}
