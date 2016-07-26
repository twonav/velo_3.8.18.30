cmd_drivers/gpu/arm/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/gpu/arm/built-in.o drivers/gpu/arm/mali/built-in.o ; scripts/mod/modpost drivers/gpu/arm/built-in.o
