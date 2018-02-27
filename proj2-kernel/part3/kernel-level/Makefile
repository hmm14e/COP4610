obj-y := sys_start_elevator.o
obj-y += sys_issue_request.o
obj-y += sys_stop_elevator.o
obj-m := elevator.o

PWD := $(shell pwd)
KDIR := /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.o *.ko *.mod.* Module.* modules.*
