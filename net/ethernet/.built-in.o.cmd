cmd_net/ethernet/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o net/ethernet/built-in.o net/ethernet/eth.o ; scripts/mod/modpost net/ethernet/built-in.o
