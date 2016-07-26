cmd_fs/devpts/devpts.o := arm-linux-gnueabihf-ld -EL    -r -o fs/devpts/devpts.o fs/devpts/inode.o ; scripts/mod/modpost fs/devpts/devpts.o
