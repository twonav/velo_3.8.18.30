cmd_drivers/hwmon/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/hwmon/built-in.o drivers/hwmon/hwmon.o ; scripts/mod/modpost drivers/hwmon/built-in.o
