ifneq ($(KERNELRELEASE),)
	obj-m := mywq.o

else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build 
	PWD = $(shell pwd)

default:
	echo $(MAKE) -C $(KDIR) M=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	make clean

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

disclean: clean
	@rm *.ko *.symvers

endif
