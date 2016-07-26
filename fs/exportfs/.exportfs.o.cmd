cmd_fs/exportfs/exportfs.o := arm-linux-gnueabihf-ld -EL    -r -o fs/exportfs/exportfs.o fs/exportfs/expfs.o ; scripts/mod/modpost fs/exportfs/exportfs.o
