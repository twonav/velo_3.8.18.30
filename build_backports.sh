#1.EXPORT REQUIRED VARIABLES

export CCPREFIX=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export KERNEL_SRC=/home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export BACK_PORTS=/home/ebosch/Kernels_IMASD/backports-3.17.1-1
export KLIB_BUILD=/home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8
export KLIB=/home/ebosch/Velo_images/kernel_modules

#2.CLEAN PREVIOUS COMPILATION
rm -rf $KLIB/lib/modules/*
rm -rf /media/ebosch/trusty/lib/modules/*

#3.BUILD KERNEL AS USUAL

cd $KERNEL_SRC
make mrproper
make twonav_velo_defconfig
#make wireless_backports_defconfig
make oldconfig
make -j4
make modules_install INSTALL_MOD_PATH=$KLIB && sync
make modules_install INSTALL_MOD_PATH=/media/ebosch/trusty && sync

#4.BUILD BACKPORTS

cd $BACK_PORTS
make mrproper
make defconfig-wifi
make oldconfig
make -j4
make install

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

#6.INSTALL MODULES

cp -r $KLIB/lib/modules/$kernel_name/updates /media/ebosch/trusty/lib/modules/$kernel_name/wireless_backports
cp $KERNEL_SRC/arch/arm/boot/zImage /media/ebosch/BOOT/zImage
cd $KERNEL_SRC
make modules_install INSTALL_MOD_PATH=/media/ebosch/trusty && sync

#7.BUILD PACKAGE
cd $KERNEL_SRC
DEB_HOST_ARCH=armhf make-kpkg -j5 --rootcmd fakeroot --arch arm --cross-compile arm-linux-gnueabihf- --initrd --zImage linux_headers linux_image
cp extras/zImage debian/linux-image-$kernel_name/etc/kernel/postinst.d/update_zImage
cp extras/uInitrd debian/linux-image-$kernel_name/etc/kernel/postinst.d/update_uInitrd
cp -r /home/ebosch/Velo_images/kernel_modules/lib/modules/$kernel_name/updates debian/linux-image-$kernel_name/lib/modules/$kernel_name/wireless_backports
dpkg --build /home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8/debian/linux-image-$kernel_name ..
dpkg --build /home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8/debian/linux-headers-$kernel_name ..
