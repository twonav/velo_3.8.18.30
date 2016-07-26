cmd_arch/arm/common/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o arch/arm/common/built-in.o arch/arm/common/firmware.o arch/arm/common/gic.o ; scripts/mod/modpost arch/arm/common/built-in.o
