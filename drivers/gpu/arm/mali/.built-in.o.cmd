cmd_drivers/gpu/arm/mali/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/gpu/arm/mali/built-in.o drivers/gpu/arm/mali/mali.o ; scripts/mod/modpost drivers/gpu/arm/mali/built-in.o
