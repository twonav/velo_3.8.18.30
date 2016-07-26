cmd_net/unix/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o net/unix/built-in.o net/unix/unix.o net/unix/unix_diag.o ; scripts/mod/modpost net/unix/built-in.o
