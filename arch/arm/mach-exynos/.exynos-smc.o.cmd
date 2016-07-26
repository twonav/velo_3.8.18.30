cmd_arch/arm/mach-exynos/exynos-smc.o := arm-linux-gnueabihf-gcc -Wp,-MD,arch/arm/mach-exynos/.exynos-smc.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float      -Wa,-march=armv7-a+sec   -c -o arch/arm/mach-exynos/exynos-smc.o arch/arm/mach-exynos/exynos-smc.S

source_arch/arm/mach-exynos/exynos-smc.o := arch/arm/mach-exynos/exynos-smc.S

deps_arch/arm/mach-exynos/exynos-smc.o := \
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

arch/arm/mach-exynos/exynos-smc.o: $(deps_arch/arm/mach-exynos/exynos-smc.o)

$(deps_arch/arm/mach-exynos/exynos-smc.o):
