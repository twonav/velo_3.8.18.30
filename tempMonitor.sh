while :

do

	temp=`cat /sys/class/thermal/thermal_zone0/temp`
	date=`date | awk '{print $4}'`
	temp=`echo "scale=2; $temp / 1000" | bc`
	echo "$date $temp ºC"

	sleep 2
done

