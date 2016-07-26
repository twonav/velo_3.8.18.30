cmd_fs/debugfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/debugfs/built-in.o fs/debugfs/debugfs.o ; scripts/mod/modpost fs/debugfs/built-in.o
