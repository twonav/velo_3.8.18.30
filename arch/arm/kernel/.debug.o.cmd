cmd_arch/arm/kernel/debug.o := arm-linux-gnueabihf-gcc -Wp,-MD,arch/arm/kernel/.debug.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o arch/arm/kernel/debug.o arch/arm/kernel/debug.S

source_arch/arm/kernel/debug.o := arch/arm/kernel/debug.S

deps_arch/arm/kernel/debug.o := \
    $(wildcard include/config/debug/semihosting.h) \
    $(wildcard include/config/debug/ll/include.h) \
    $(wildcard include/config/mmu.h) \
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
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/hwcap.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi/asm/hwcap.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/opcodes-virt.h \
  /home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/asm/opcodes.h \
    $(wildcard include/config/cpu/endian/be32.h) \
  include/linux/stringify.h \
  arch/arm/mach-exynos/include/mach/debug-macro.S \
    $(wildcard include/config/debug/s3c/uart.h) \
  arch/arm/mach-exynos/include/mach/map.h \
  arch/arm/plat-samsung/include/plat/map-base.h \
  arch/arm/plat-samsung/include/plat/map-s5p.h \
  arch/arm/plat-samsung/include/plat/map-s3c.h \
    $(wildcard include/config/cpu/s3c2412.h) \
    $(wildcard include/config/cpu/s3c2413.h) \
    $(wildcard include/config/cpu/s3c2412/only.h) \
  arch/arm/plat-samsung/include/plat/debug-macro.S \
  arch/arm/plat-samsung/include/plat/regs-serial.h \

arch/arm/kernel/debug.o: $(deps_arch/arm/kernel/debug.o)

$(deps_arch/arm/kernel/debug.o):
