cmd_net/rxrpc/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o net/rxrpc/built-in.o net/rxrpc/af-rxrpc.o ; scripts/mod/modpost net/rxrpc/built-in.o
