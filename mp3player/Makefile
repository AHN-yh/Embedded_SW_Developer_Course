INCLUDE=/nfs/rootfs/mp3player/include

all:
	arm-none-linux-gnueabi-gcc mp3player.c -I$(INCLUDE) include/fbutils.c -o mp3player -lts

install:
	cp a.out /nfsroot/root

clean:
	rm -rf *.o
	rm -rf a.out

