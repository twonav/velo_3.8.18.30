#!/bin/bash

if [ "$1" == "help" ]
then
	echo Usage:
	echo - Run \"./configure.sh\" for ext2 root file system
	echo - Run \"./configure.sh ubi\" for ubi root file system
else
	cd build
	sudo multistrap -a armhf -f multistrap.conf
	sudo cp /usr/bin/qemu-arm-static target-rootfs/usr/bin
	sudo mount -o bind /dev/ target-rootfs/dev/
	sudo mount -t proc none target-rootfs/proc
	sudo LC_ALL=C LANGUAGE=C LANG=C chroot target-rootfs dpkg --configure -a
	# Clean file system for storage in NAND flash
	if [ "$1" == "ubi" ]
	then
		sudo chroot target-rootfs localepurge
		sudo chroot target-rootfs rm -rf /usr/share/doc/
		sudo chroot target-rootfs rm -rf /usr/share/doc-base/
	fi
	sudo chroot target-rootfs passwd
	sudo chroot target-rootfs apt-get clean
	sudo chroot target-rootfs /var/lib/dpkg/info/dash.preinst install
	sudo chroot target-rootfs dpkg --configure -a
	sudo rm target-rootfs/usr/bin/qemu-arm-static
	sudo umount -f target-rootfs/dev/
	sudo umount -l target-rootfs/dev/
	sudo umount -f target-rootfs/proc/
	sudo umount -l target-rootfs/proc/
	sudo umount -f build/target-rootfs/run/shm
	sudo umount -l build/target-rootfs/run/shm
fi
