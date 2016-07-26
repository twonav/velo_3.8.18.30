cmd_fs/fat/vfat.o := arm-linux-gnueabihf-ld -EL    -r -o fs/fat/vfat.o fs/fat/namei_vfat.o ; scripts/mod/modpost fs/fat/vfat.o
