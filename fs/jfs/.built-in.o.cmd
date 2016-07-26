cmd_fs/jfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/jfs/built-in.o fs/jfs/jfs.o ; scripts/mod/modpost fs/jfs/built-in.o
