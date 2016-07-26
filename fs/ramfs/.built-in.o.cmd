cmd_fs/ramfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/ramfs/built-in.o fs/ramfs/ramfs.o ; scripts/mod/modpost fs/ramfs/built-in.o
