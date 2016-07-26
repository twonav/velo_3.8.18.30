cmd_drivers/input/keyboard/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/input/keyboard/built-in.o drivers/input/keyboard/atkbd.o ; scripts/mod/modpost drivers/input/keyboard/built-in.o
