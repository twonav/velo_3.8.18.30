cmd_drivers/md/md-mod.o := arm-linux-gnueabihf-ld -EL    -r -o drivers/md/md-mod.o drivers/md/md.o drivers/md/bitmap.o ; scripts/mod/modpost drivers/md/md-mod.o
