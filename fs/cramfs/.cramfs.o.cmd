cmd_fs/cramfs/cramfs.o := arm-linux-gnueabihf-ld -EL    -r -o fs/cramfs/cramfs.o fs/cramfs/inode.o fs/cramfs/uncompress.o ; scripts/mod/modpost fs/cramfs/cramfs.o
