obj-m += prm.o
prm-objs := prm_module.o prm_hook.o prm_netlink.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

ins:
	sudo insmod prm.ko

rm:
	sudo rmmod prm

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean