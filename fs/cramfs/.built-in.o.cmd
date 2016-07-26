cmd_fs/cramfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/cramfs/built-in.o fs/cramfs/cramfs.o ; scripts/mod/modpost fs/cramfs/built-in.o
