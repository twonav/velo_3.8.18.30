#!/bin/sh

convert ../../../../../CompeGPS/versions/TwoNav/a_ultima/Splashes/Velo/pageBoot.bmp pageBootVelo.png
pngtopnm pageBootVelo.png | ppmquant 224 | pnmtoplainpnm > logo_twonav_small_clut224.ppm 

convert ../../../../../CompeGPS/versions/TwoNav/a_ultima/Splashes/VeloOS/pageBoot.bmp pageBootVeloOs.png
pngtopnm pageBootVeloOs.png | ppmquant 224 | pnmtoplainpnm > logo_os_small_clut224.ppm 

convert ../../../../../CompeGPS/versions/TwoNav/a_ultima/Splashes/AventuraLinux/pageBoot.bmp pageBootAventura.png
pngtopnm pageBootAventura.png | ppmquant 224 | pnmtoplainpnm > logo_twonav_big_clut224.ppm 

convert ../../../../../CompeGPS/versions/TwoNav/a_ultima/Splashes/AventuraLinuxOrdnanceSurvey/pageBoot.bmp pageBootAventuraOs.png
pngtopnm pageBootAventuraOs.png | ppmquant 224 | pnmtoplainpnm > logo_os_big_clut224.ppm 

rm -rf pageBoot*


