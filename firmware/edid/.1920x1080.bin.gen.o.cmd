cmd_firmware/edid/1920x1080.bin.gen.o := arm-linux-gnueabihf-gcc -Wp,-MD,firmware/edid/.1920x1080.bin.gen.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o firmware/edid/1920x1080.bin.gen.o firmware/edid/1920x1080.bin.gen.S

source_firmware/edid/1920x1080.bin.gen.o := firmware/edid/1920x1080.bin.gen.S

deps_firmware/edid/1920x1080.bin.gen.o := \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \

firmware/edid/1920x1080.bin.gen.o: $(deps_firmware/edid/1920x1080.bin.gen.o)

$(deps_firmware/edid/1920x1080.bin.gen.o):
