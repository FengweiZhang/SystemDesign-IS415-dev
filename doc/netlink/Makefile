obj-m:=my_netlink.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	gcc ./my_user.c -o ./my_user.out

clean:
	$(RM) -rf .*.cmd *.mod.c *.o *.ko .tmp* *.mod *.symvers *.order *.out