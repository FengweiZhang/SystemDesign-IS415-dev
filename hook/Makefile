obj-m:=my_hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

ins:
	insmod ./my_hook.ko
	dmesg

rm:
	rmmod my_hook


clean:
	$(RM) -rf .*.cmd *.mod.c *.o *.ko .tmp* *.mod *.symvers *.order *.out