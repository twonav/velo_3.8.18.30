cmd_drivers/hid/i2c-hid/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/hid/i2c-hid/built-in.o drivers/hid/i2c-hid/i2c-hid.o ; scripts/mod/modpost drivers/hid/i2c-hid/built-in.o
