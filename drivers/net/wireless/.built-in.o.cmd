cmd_drivers/net/wireless/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/net/wireless/built-in.o drivers/net/wireless/ti/built-in.o ; scripts/mod/modpost drivers/net/wireless/built-in.o
