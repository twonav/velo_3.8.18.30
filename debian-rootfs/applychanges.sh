#!/bin/sh

ORIGIN=`pwd`

if [ "$1" == "help" ]
then
	echo Usage:
	echo - Run \"./configure.sh\" for ext2 root file system
	echo - Run \"./configure.sh ubi\" for ubi root file system
else

	echo
	echo "Adding new files to rootfs"
	echo


	if [ "$1" == "ubi" ]
	then
		sudo cp -R work/Sysconfig_ubi/* build/target-rootfs
		sudo mv build/target-rootfs/etc/fstab_ubi build/target-rootfs/etc/fstab
	else
		sudo cp -R work/Sysconfig/* build/target-rootfs
	fi
fi

cd $ORIGIN
