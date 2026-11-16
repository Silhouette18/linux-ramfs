obj-m := my_ramfs.o

my_ramfs-y := super.o inode.o file.o

KDIR := /home/kali/kernel_workspace/buildroot-2024.02.2/output/build/linux-6.1.44/
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
                                                                                                                                                                                                                                           
