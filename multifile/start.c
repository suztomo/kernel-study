#include <linux/module.h>
#include <linux/kernel.h>

#include "hogefunc.h"

int init_module(void)
{
  print_hello();
  return 0;
}
