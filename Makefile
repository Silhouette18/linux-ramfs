KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := my_ramfs.o
my_ramfs-y := super.o inode.o file.o

all:
    $(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
    $(MAKE) -C $(KDIR) M=$(PWD) clean
