#!/bin/bash

if [ "$1" == "ubi" ]
then
	cp config/multistrap_ubi.conf build/multistrap.conf
elif [ "$1" == "help" ]
then
	echo Usage:
	echo - Run \"./configure.sh\" for ext2 root file system
	echo - Run \"./configure.sh ubi\" for ubi root file system
else
	cp config/multistrap.conf build
fi
