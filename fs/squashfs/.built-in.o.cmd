cmd_fs/squashfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/squashfs/built-in.o fs/squashfs/squashfs.o ; scripts/mod/modpost fs/squashfs/built-in.o
