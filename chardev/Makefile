TARGET=chardev
obj-m += $(TARGET).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

check-syntax:
	LANG=C make -C /lib/modules/$(shell uname -r)/build M=/home/suzuki/kernel-study/$(TARGET) modules

install:
	sudo insmod $(TARGET).ko

uninstall:
	sudo rmmod $(TARGET)
