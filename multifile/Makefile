obj-m += startstop.o
startstop-objs := start.o stop.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


TARGET=startstop

install:
	sudo insmod $(TARGET).ko

uninstall:
	sudo rmmod $(TARGET)
