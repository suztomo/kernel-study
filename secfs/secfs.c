/*
 * Security file system.
 *
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/security.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>


#define DIR_NAME "hp"

MODULE_AUTHOR("SUZUKI Tomohiro");
MODULE_LICENSE("GPL");

struct dentry * hp_dir_entry;

int init_module(void)
{
  hp_dir_entry = securityfs_create_dir(DIR_NAME, NULL);
  if (!hp_dir_entry) {
    printk(KERN_ALERT "failed securityfs_create_dir.\n");
  }
  if ((int)hp_dir_entry == -ENODEV) {
    printk(KERN_ALERT "securityfs is not enabled in this machine.\n");
    hp_dir_entry = NULL;
  }
  return 0;
}

void cleanup_module(void)
{
  if (hp_dir_entry) {
    securityfs_remove(hp_dir_entry);
  }
}

