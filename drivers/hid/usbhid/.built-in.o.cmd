cmd_drivers/hid/usbhid/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/hid/usbhid/built-in.o drivers/hid/usbhid/usbhid.o ; scripts/mod/modpost drivers/hid/usbhid/built-in.o
