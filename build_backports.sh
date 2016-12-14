# http://confluence.compegps.com/display/TV/Cross+Compiling+Wireless+Backports

# Requirements:
# apt-get install kernel-package

if [[ ! $(whoami) =~ "root" ]]; then
echo ""
echo "**********************************"
echo "*** This should be run as root ***"
echo "**********************************"
echo ""
exit
fi

if [[ -z $1 ]]; then
echo "Usage: ./build_backports.sh <USER>"
echo " <USER> is the name of home subfolder (ebosch, dnieto, etc)"
exit
fi

HOMEUSERFOLDER=$1

#1.EXPORT REQUIRED VARIABLES

export CCPREFIX=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export KERNEL_SRC=/home/$HOMEUSERFOLDER/Kernels_IMASD/Clickarm_Kernel_3.8
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export BACK_PORTS=/home/$HOMEUSERFOLDER/Kernels_IMASD/backports-3.17.1-1
export KLIB_BUILD=/home/$HOMEUSERFOLDER/Kernels_IMASD/Clickarm_Kernel_3.8
export KLIB=/home/$HOMEUSERFOLDER/Velo_images/kernel_modules

echo "**** STEP 1 END EXPORT REQUIRED VARIABLES ****"

#2.CLEAN PREVIOUS COMPILATION
rm -rf $KLIB/lib/modules/*
rm -rf /media/$HOMEUSERFOLDER/trusty/lib/modules/*

echo "**** STEP 2 END CLEAN PREVIOUS COMPILATION ****"

#3.BUILD KERNEL AS USUAL

cd $KERNEL_SRC
make mrproper
make twonav_velo_defconfig
#make wireless_backports_defconfig
make oldconfig
make -j4
make modules_install INSTALL_MOD_PATH=$KLIB && sync
make modules_install INSTALL_MOD_PATH=/media/$HOMEUSERFOLDER/trusty && sync

echo "**** STEP 3 END BUILD KERNEL AS USUAL ****"

#4.BUILD BACKPORTS

cd $BACK_PORTS
make mrproper
make defconfig-wifi
make oldconfig
make -j4
make install

echo "**** STEP 4 END BUILD BACKPORTS ****"

#5.CHECK NEW KERNEL NAME

i=0
kernel_name=""

for directory in $(find $KLIB/lib/modules/ -type d); 
do
        i=$((i+1))
        if [ $i -eq 2  ]; then
                kernel_name=${directory##*/}
        fi
done

echo "Current kernel compilation: $kernel_name"

echo "**** STEP 5 END CHECK NEW KERNEL NAME ****"

#6.INSTALL MODULES

cp -r $KLIB/lib/modules/$kernel_name/updates /media/$HOMEUSERFOLDER/trusty/lib/modules/$kernel_name/wireless_backports
cp $KERNEL_SRC/arch/arm/boot/zImage /media/$HOMEUSERFOLDER/BOOT/zImage
cd $KERNEL_SRC
make modules_install INSTALL_MOD_PATH=/media/$HOMEUSERFOLDER/trusty && sync

echo "**** STEP 6 END INSTALL MODULES ****"

#7.BUILD PACKAGE
cd $KERNEL_SRC
DEB_HOST_ARCH=armhf make-kpkg --revision=1.0.0Velo -j5 --rootcmd fakeroot --arch arm --cross-compile arm-linux-gnueabihf- --initrd --zImage linux_headers linux_image
cp extras/update_zImage debian/linux-image-$kernel_name/etc/kernel/postinst.d/update_zImage
cp extras/update_uInitrd debian/linux-image-$kernel_name/etc/kernel/postinst.d/update_uInitrd
cp -r /home/$HOMEUSERFOLDER/Velo_images/kernel_modules/lib/modules/$kernel_name/updates debian/linux-image-$kernel_name/lib/modules/$kernel_name/wireless_backports
dpkg --build /home/$HOMEUSERFOLDER/Kernels_IMASD/Clickarm_Kernel_3.8/debian/linux-image-$kernel_name ..
dpkg --build /home/$HOMEUSERFOLDER/Kernels_IMASD/Clickarm_Kernel_3.8/debian/linux-headers-$kernel_name ..

echo "**** STEP 7 END ****"


