cmd_drivers/dma/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/dma/built-in.o drivers/dma/dmaengine.o drivers/dma/pl330.o ; scripts/mod/modpost drivers/dma/built-in.o
