#1.EXPORT REQUIRED VARIABLES

export CCPREFIX=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export KERNEL_SRC=/home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-
export BACK_PORTS=/home/ebosch/Kernels_IMASD/backports-3.17.1-1
export KLIB_BUILD=/home/ebosch/Kernels_IMASD/Clickarm_Kernel_3.8
export KLIB=/home/ebosch/Velo_images/kernel_modules/


#2.BUILD KERNEL AS USUAL

cd $KERNEL_SRC
make mrproper
#make twonav_velo_defconfig
make wireless_backports_defconfig
make oldconfig
make -j4



#3.BUILD BACKPORTS

cd $BACK_PORTS
make mrproper
make defconfig-wifi
make oldconfig
make -j4
make install


#4.INSTALL MODULES

cp -r $KLIB/lib/modules/3.8.13.30TwoNav/updates /media/ebosch/trusty/lib/modules/3.8.13.30TwoNav/wireless_backports
cp $KERNEL_SRC/arch/arm/boot/zImage /media/ebosch/BOOT/zImage 
cd $KERNEL_SRC
make modules_install INSTALL_MOD_PATH=/media/ebosch/trusty && sync

