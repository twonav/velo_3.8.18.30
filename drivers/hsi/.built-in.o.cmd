cmd_drivers/hsi/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/hsi/built-in.o drivers/hsi/clients/built-in.o ; scripts/mod/modpost drivers/hsi/built-in.o
