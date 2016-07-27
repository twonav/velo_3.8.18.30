#/bin/bash

bash ./rebuilld.sh
if [ $? -eq 0 ]; then
	bash ./kernel_a_sd.sh
else
	exit 1
fi
exit 0

