cmd_lib/bcd.o := arm-linux-gnueabihf-gcc -Wp,-MD,lib/.bcd.o.d  -nostdinc -isystem /usr/lib/gcc-cross/arm-linux-gnueabihf/5/include  -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/arturo/clickarm_3.8.18.30_IMASD/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/arturo/clickarm_3.8.18.30_IMASD/include/uapi -Iinclude/generated/uapi -include /home/arturo/clickarm_3.8.18.30_IMASD/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-exynos/include -Iarch/arm/plat-samsung/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -std=gnu89 -O2 -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(bcd)"  -D"KBUILD_MODNAME=KBUILD_STR(bcd)" -c -o lib/bcd.o lib/bcd.c

source_lib/bcd.o := lib/bcd.c

deps_lib/bcd.o := \
  include/linux/bcd.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc5.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  include/linux/export.h \
    $(wildcard include/config/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \

lib/bcd.o: $(deps_lib/bcd.o)

$(deps_lib/bcd.o):
