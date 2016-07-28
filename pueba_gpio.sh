#!/bin/bash

#-----------------------------------------------------------------
# Prueba gpios para velo. Perpetrado por Arturo Sapiña. Julio 2016
#-----------------------------------------------------------------


#-----------------INICIALIZAR TODO-------------------------------

#----------------GPRS--------------------
echo 57 > /sys/class/gpio/export #!on/off
echo out > /sys/class/gpio/gpio57/direction

echo 62 > /sys/class/gpio/export #status
echo in > /sys/class/gpio/gpio57/direction

echo 51 > /sys/class/gpio/export #pwrkey
echo out > /sys/class/gpio/gpio57/direction

#----------------GPS--------------------
echo 81 > /sys/class/gpio/export #on/off
echo out > /sys/class/gpio/gpio57/direction

echo 84 > /sys/class/gpio/export #status
echo in > /sys/class/gpio/gpio57/direction

echo 59 > /sys/class/gpio/export #reset
echo out > /sys/class/gpio/gpio57/direction

#---------------WLAN----------------------
echo 135 > /sys/class/gpio/export #wlan-enable
echo out > /sys/class/gpio/gpio57/direction

echo 128 > /sys/class/gpio/export #bt-enable
echo in > /sys/class/gpio/gpio57/direction

#-------------botones--------------------

echo 111 > /sys/class/gpio/export #topleft
echo in > /sys/class/gpio/gpio57/direction

echo 78 > /sys/class/gpio/export #topright
echo in > /sys/class/gpio/gpio57/direction

echo 123 > /sys/class/gpio/export #botright
echo in > /sys/class/gpio/gpio57/direction

echo 132 > /sys/class/gpio/export #botleft
echo in > /sys/class/gpio/gpio57/direction

#---------------otros-------------------

echo 10 > /sys/class/gpio/export #3v3 pow enable
echo out > /sys/class/gpio/gpio57/direction


#--------------------TESTS--------------------

#GPRS

echo " probando GPRS...."
echo 0 > /sys/class/gpio/gpio57/value
echo 1 > /sys/class/gpio/gpio51/value 
sleep 1.2
if [ $(cat /sys/class/gpio/gpio62/value) -eq 1 ]; then
	echo " parece que el GPRS funciona"
else
	echo " el GPRS no responde "
fi

#GPS

echo " probando GPS...."
echo 1 > /sys/class/gpio/gpio81/value 
sleep 1.2
if [ $(cat /sys/class/gpio/gpio84/value) -eq 1 ]; then
        echo " parece que el GPS funciona"
else
        echo " el GPS no responde "
fi

exit 0

