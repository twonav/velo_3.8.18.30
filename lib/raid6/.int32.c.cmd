cmd_lib/raid6/int32.c := awk -f/home/arturo/clickarm_3.8.18.30_IMASD/lib/raid6/unroll.awk -vN=32 < lib/raid6/int.uc > lib/raid6/int32.c || ( rm -f lib/raid6/int32.c && exit 1 )
