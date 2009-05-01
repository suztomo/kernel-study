#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <sys/mman.h>
#include <linux/unistd.h>	/* The list of system calls */



int main(int argc, char **argv) {
  int fd;
  fd = open("/dev/mem", O_RDWR);
  void *vaddr_dm;
  unsigned long sys_call_table = 0xc042ca00;
  long sys_call_table_offset;
  sys_call_table_offset = sys_call_table - 0xc0000000;
  sys_call_table_offset = sys_call_table_offset & (~(4096-1));
  size_t memsize = 4096;
  //  memsize = 4096 * 100;
  printf("%x\n", memsize);
  printf("%x\n", sys_call_table);
  printf("%x\n", sys_call_table_offset);

  if (fd < 0) {
    perror("fd");
    return 0;
  }
  //  sys_call_table_offset = 0;
  vaddr_dm = mmap(NULL, memsize,
                  PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                  (void **)sys_call_table_offset);
  if ((int)vaddr_dm == -1) {
    perror("mmap");
    return 0;
  }

  void **another_sys_table;
  another_sys_table = vaddr_dm + sys_call_table_offset;
  void *bkup;
  bkup = another_sys_table + __NR_open;
  printf("vaddr_dm %p\n", vaddr_dm);
  printf("syscall_offset %x\n", sys_call_table_offset);
  printf("open %p\n", bkup);
  printf("open %p\n", another_sys_table[__NR_open]);
  //  another_sys_table[__NR_open] = (void *)1;
  //printf("open %p after modification\n", sys_call_table[__NR_open]);
  //another_sys_table[__NR_open] = bkup;

  //  printf("%p\n", bkup);

  munmap(vaddr_dm, memsize);
  close(fd);

  return 0;
}
