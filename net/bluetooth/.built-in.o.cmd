cmd_net/bluetooth/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o net/bluetooth/built-in.o net/bluetooth/bluetooth.o ; scripts/mod/modpost net/bluetooth/built-in.o
