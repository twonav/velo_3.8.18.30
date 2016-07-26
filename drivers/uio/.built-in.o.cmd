cmd_drivers/uio/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/uio/built-in.o drivers/uio/uio.o ; scripts/mod/modpost drivers/uio/built-in.o
