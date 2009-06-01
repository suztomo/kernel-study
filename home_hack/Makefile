obj-m += home_hack.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

check-syntax:
	LANG=C make -C /lib/modules/2.6.27.10/build M=/home/suzuki/kernel-study/open_hack modules
