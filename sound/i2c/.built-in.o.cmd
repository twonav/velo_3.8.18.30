cmd_sound/i2c/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o sound/i2c/built-in.o sound/i2c/other/built-in.o ; scripts/mod/modpost sound/i2c/built-in.o
