#!/bin/bash

make -j6 ARCH=arm zImage
if [ $? -eq 0 ]; then
	echo "Kernel compilado"
else
	echo "Kernel a la mierda"
	exit 1
fi
make -j6 modules ARCH=arm
if [ $? -eq 0 ]; then
        echo "modulos compilados"
else
	echo "modulos a la mierda"
        exit 1
fi
make modules_install INSTALL_MOD_PATH=./modules ARCH=arm
if [ $? -eq 0 ]; then
        echo "modulos instalados"
else
		echo "modulos no instalados"
        exit 1
fi
exit 0
