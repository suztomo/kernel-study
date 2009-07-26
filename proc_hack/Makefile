TARGET=proc_hack
obj-m +=$(TARGET).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

check-syntax:
	LANG=C make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	sudo insmod $(TARGET).ko pid_array=`pgrep sshd` node_array=1

uninstall:
	sudo rmmod $(TARGET)
