#include <linux/module.h>
#include <linux/kernel.h>

void print_hello(void)
{
  printk(KERN_INFO "Hello World!!!!!!!\n");
}
