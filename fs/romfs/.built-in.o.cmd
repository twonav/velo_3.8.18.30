cmd_fs/romfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/romfs/built-in.o fs/romfs/romfs.o ; scripts/mod/modpost fs/romfs/built-in.o
