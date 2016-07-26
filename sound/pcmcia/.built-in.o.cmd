cmd_sound/pcmcia/built-in.o :=  arm-linux-gnueabihf-ld -EL    -r -o sound/pcmcia/built-in.o sound/pcmcia/vx/built-in.o sound/pcmcia/pdaudiocf/built-in.o ; scripts/mod/modpost sound/pcmcia/built-in.o
