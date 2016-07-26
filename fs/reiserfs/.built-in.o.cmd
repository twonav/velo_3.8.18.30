cmd_fs/reiserfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/reiserfs/built-in.o fs/reiserfs/reiserfs.o ; scripts/mod/modpost fs/reiserfs/built-in.o
