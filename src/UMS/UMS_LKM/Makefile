KDIR = /lib/modules/$(shell uname -r)/build
obj-m += ums.o
ums-objs := ums_LKM.o ums_hashtable.o ums_proc.o

all:
	make -C $(KDIR) M=$(PWD) modules 

clean:
	make -C $(KDIR) M=$(PWD) clean
