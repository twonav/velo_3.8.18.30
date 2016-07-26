cmd_fs/btrfs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/btrfs/built-in.o fs/btrfs/btrfs.o ; scripts/mod/modpost fs/btrfs/built-in.o
