#!/bin/bash
echo "-------------------------------------------------------"
echo "preparando tu pc para todo lo que viene................"
echo "-------------------------------------------------------"

VERSION=$(cat /etc/lsb-release|grep RELEASE)
if [ $VERSION == "DISTRIB_RELEASE=14.04" ]
	then
		sudo apt-get install realpath
		wget http://mirrors.kernel.org/ubuntu/pool/universe/m/multistrap/multistrap_2.1.6ubuntu3_all.deb
		sudo dpkg -i multistrap_2.1.6ubuntu3_all.deb
		sudo rm multistrap_2.1.6ubuntu3_all.deb
	else 
		sudo apt-get install multistrap
fi

sudo apt-get install qemu qemu-user-static binfmt-support dpkg-cross

mkdir -p build/target-rootfs
cd work

echo
echo "Extracting personalization files"
echo

if [ "$1" == "ubi" ]
then
	sudo tar jvxf ../sources/Sysconfig_ubi.tar.bz2
	mv Sysconfig Sysconfig_ubi
elif [ "$1" == "help" ]
then
	echo Usage:
	echo - Run \"./prepare.sh\" for ext2 root file system
	echo - Run \"./prepare.sh ubi\" for ubi root file system
else
	sudo tar jvxf ../sources/Sysconfig.tar.bz2
fi

cd ..
