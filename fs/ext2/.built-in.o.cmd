cmd_fs/ext2/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/ext2/built-in.o fs/ext2/ext2.o ; scripts/mod/modpost fs/ext2/built-in.o
