#!/bin/sh

echo
echo Cleaning debian rootfs
echo

sudo umount build/target-rootfs/run/shm
sudo umount target-rootfs/dev
sudo umount target-rootfs/proc

sudo rm -rf build/*
if [ "$1" == "distclean" ]
then
	sudo rm -rf work/*
	sudo rm -rf target/*
fi
