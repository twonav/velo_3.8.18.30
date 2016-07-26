#!/bin/bash


ORIGIN=$PWD

if [ "$1" == "help" ]
then
	echo Usage:
	echo - Run \"./configure.sh\" for ext2 root file system
	echo - Run \"./configure.sh ubi\" for ubi root file system
elif [ "$1" == "ubi" ]
then
	echo
	echo Copying output files to target dir
	echo

	cd build 
	sudo mkfs.ubifs -r target-rootfs/ -m 0x800 -e 0x1f000 -c 2048 -o ../target/rootfs.ubifs
else
	echo
	echo Copying output files to target dir
	echo

	cd build/target-rootfs
#	sudo tar cvpjf  ../../target/rootfs.tar.bz2 .
fi

cd $ORIGIN
