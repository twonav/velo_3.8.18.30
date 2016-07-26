cmd_fs/romfs/romfs.o := arm-linux-gnueabihf-ld -EL    -r -o fs/romfs/romfs.o fs/romfs/storage.o fs/romfs/super.o ; scripts/mod/modpost fs/romfs/romfs.o
