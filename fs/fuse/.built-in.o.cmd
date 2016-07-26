cmd_fs/fuse/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/fuse/built-in.o fs/fuse/fuse.o ; scripts/mod/modpost fs/fuse/built-in.o
