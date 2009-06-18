#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>



int main ()
{

  struct ifreq ifr[10];
  struct ifconf ifc;
  int fd;
  int nifs, i,j;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifc.ifc_len = sizeof(ifr);

  ifc.ifc_ifcu.ifcu_buf = (void*)ifr;


  ioctl(fd, SIOCGIFCONF, &ifc);

  nifs = ifc.ifc_len / sizeof(struct ifreq);

  for (i=0; i<nifs; ++i) {
    printf("%s\n", ifr[i].ifr_name);
    for (j=0; j<14; ++j) {
      //#define	ifr_addr	ifr_ifru.ifru_addr
      printf("%02x", ifr[i].ifr_addr.sa_data[j] & 0xff);
    }
    printf("\n");
  }

  close(fd);

  return 0;
}
