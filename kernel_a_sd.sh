#/bin/bash

cp arch/arm/boot/zImage /media/$USER/BOOT/zImage
if [ $? -eq 0 ]; then
	echo "zImage ok"
else
        echo "zImage no se ha copiado bien"
        exit 1
fi
sudo rsync -avc ./modules/lib/. /media/$USER/trusty/lib.
if [ $? -eq 0 ]; then
	echo "modulos listos"
else
	echo "los modulos no estan bien copiados"
	exit 1
fi
exit 0
