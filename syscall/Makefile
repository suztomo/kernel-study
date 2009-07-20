TARGET=syscall
obj-m += $(TARGET).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod $(TARGET).ko uid=1000

uninstall:
	sudo rmmod $(TARGET)
