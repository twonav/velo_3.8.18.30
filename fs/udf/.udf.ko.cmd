cmd_fs/udf/udf.ko := arm-linux-gnueabihf-ld -EL -r  -T /home/arturo/clickarm_3.8.18.30_IMASD/scripts/module-common.lds --build-id  -o fs/udf/udf.ko fs/udf/udf.o fs/udf/udf.mod.o
