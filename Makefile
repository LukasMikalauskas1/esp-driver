obj-m = espdrv.o

KV = $(shell uname -r)

all:
	make -C /lib/modules/$(KV)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KV)/build M=$(PWD) clean
