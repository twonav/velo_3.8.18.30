cmd_fs/ntfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/ntfs/built-in.o fs/ntfs/ntfs.o ; scripts/mod/modpost fs/ntfs/built-in.o
