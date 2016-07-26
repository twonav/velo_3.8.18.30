cmd_fs/isofs/isofs.ko := arm-linux-gnueabihf-ld -EL -r  -T /home/arturo/clickarm_3.8.18.30_IMASD/scripts/module-common.lds --build-id  -o fs/isofs/isofs.ko fs/isofs/isofs.o fs/isofs/isofs.mod.o
