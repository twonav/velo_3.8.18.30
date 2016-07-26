cmd_drivers/usb/mon/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/usb/mon/built-in.o drivers/usb/mon/usbmon.o ; scripts/mod/modpost drivers/usb/mon/built-in.o
