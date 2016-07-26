cmd_drivers/usb/serial/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/usb/serial/built-in.o drivers/usb/serial/usbserial.o ; scripts/mod/modpost drivers/usb/serial/built-in.o
