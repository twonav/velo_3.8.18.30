cmd_drivers/clk/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o drivers/clk/built-in.o drivers/clk/clk-devres.o drivers/clk/clkdev.o ; scripts/mod/modpost drivers/clk/built-in.o
