cmd_fs/cifs/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o fs/cifs/built-in.o fs/cifs/cifs.o ; scripts/mod/modpost fs/cifs/built-in.o
