cmd_fs/hfsplus/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/hfsplus/built-in.o fs/hfsplus/hfsplus.o ; scripts/mod/modpost fs/hfsplus/built-in.o
