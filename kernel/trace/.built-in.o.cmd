cmd_kernel/trace/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o kernel/trace/built-in.o kernel/trace/power-traces.o kernel/trace/rpm-traces.o ; scripts/mod/modpost kernel/trace/built-in.o
