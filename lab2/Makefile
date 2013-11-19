obj-m := clock.o
PWD := $(shell pwd)

default:
	make -C /lib/modules/3.11.0-12-generic/build SUBDIRS=$(PWD) modules
