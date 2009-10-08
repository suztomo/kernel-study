#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/syscalls.h> /* sys_close */
#include <linux/module.h>	/* Specifically, a module, */
#include <linux/moduleparam.h>	/* which will have params */
#include <linux/unistd.h>	/* The list of system calls */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>

static LIST_HEAD(my_object_list);

struct my_object {
  struct list_head list;
  /* Time when it recorded from when it started to record */
  long int sec;
  /* tty device name, distinguishing ttys in a honeypot node */
  char tty_name[6];
};

int init_module(void)
{
  struct list_head *p;
  struct my_object *myobj;
  int i;
  printk(KERN_INFO "list append test.\n");

  /*
    GFP_KERNEL : If there is no memory available, it sleeps.
    GFP_ATOMIC : Do not sleep. If no memory available, it fails.
   */
  for (i=0; i<10000; ++i) {
    myobj = kmalloc(sizeof(struct my_object), GFP_KERNEL);
    myobj->sec = i;
    strncpy(myobj->tty_name, "tty", 5);
    list_add_tail(&myobj->list, &my_object_list);
  }

  list_for_each(p, &my_object_list) {
    myobj = list_entry(p, struct my_object, list);
    printk("*** name %s : %ld\n", myobj->tty_name, myobj->sec);
  }

  return 0;
}

void cleanup_module(void)
{
  struct my_object *myobj;
  /*
  struct list_head *next;
  next = head.next;*/
  int i = 0;
  printk("*** start to delete\n");
  while(!list_empty(&my_object_list)) {
    myobj = list_entry(my_object_list.next, struct my_object, list);
    printk("*** deleting %d\n", i++);
    list_del(&myobj->list);
    kfree(myobj);
  }

  printk(KERN_INFO "Goodbye.\n");
}
