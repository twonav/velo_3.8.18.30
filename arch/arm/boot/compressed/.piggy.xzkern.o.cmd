cmd_arch/arm/boot/compressed/piggy.xzkern.o := arm-linux-gnueabihf-gcc -Wp,-MD,arch/arm/boot/compressed/.piggy.xzkern.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float     -DZIMAGE     -c -o arch/arm/boot/compressed/piggy.xzkern.o arch/arm/boot/compressed/piggy.xzkern.S

source_arch/arm/boot/compressed/piggy.xzkern.o := arch/arm/boot/compressed/piggy.xzkern.S

deps_arch/arm/boot/compressed/piggy.xzkern.o := \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \

arch/arm/boot/compressed/piggy.xzkern.o: $(deps_arch/arm/boot/compressed/piggy.xzkern.o)

$(deps_arch/arm/boot/compressed/piggy.xzkern.o):