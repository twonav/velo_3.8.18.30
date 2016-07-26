cmd_arch/arm/plat-samsung/s5p-sleep.o := arm-linux-gnueabihf-gcc -Wp,-MD,arch/arm/plat-samsung/.s5p-sleep.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o arch/arm/plat-samsung/s5p-sleep.o arch/arm/plat-samsung/s5p-sleep.S

source_arch/arm/plat-samsung/s5p-sleep.o := arch/arm/plat-samsung/s5p-sleep.S

deps_arch/arm/plat-samsung/s5p-sleep.o := \
    $(wildcard include/config/cache/l2x0.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/linkage.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/asm-offsets.h \
  include/generated/asm-offsets.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/hardware/cache-l2x0.h \
    $(wildcard include/config/of.h) \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  arch/arm/include/generated/asm/errno.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/include/uapi/asm-generic/errno.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/include/uapi/asm-generic/errno-base.h \

arch/arm/plat-samsung/s5p-sleep.o: $(deps_arch/arm/plat-samsung/s5p-sleep.o)

$(deps_arch/arm/plat-samsung/s5p-sleep.o):
