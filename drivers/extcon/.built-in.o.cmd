cmd_drivers/extcon/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/extcon/built-in.o drivers/extcon/extcon-class.o ; scripts/mod/modpost drivers/extcon/built-in.o
