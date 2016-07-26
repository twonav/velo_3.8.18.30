cmd_fs/xfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/xfs/built-in.o fs/xfs/xfs.o ; scripts/mod/modpost fs/xfs/built-in.o
