
function usage ()
{
    echo "Usage: <option=1> `basename $0` <kernel_config_file>"
	echo "options:"
	echo "IP_TABLES - add ip tables support"
	echo "NO_DEVICE_TREE - add support for non device tree compilation"
	echo "BT_HCI - add support for BT HCI"
	exit 1
}

base_config=(
    CONFIG_WLAN=y
    CONFIG_WIRELESS=y
    CONFIG_KEYS=y
    CONFIG_SECURITY=y
    CONFIG_CRYPTO=y
    CONFIG_WIRELESS_EXT=y
    CONFIG_CRYPTO_ARC4=y
    CONFIG_CRYPTO_ECB=y
    CONFIG_CRYPTO_AES=y
    CONFIG_CRYPTO_MICHAEL_MIC=y
    CONFIG_CRYPTO_CCM=y
    CONFIG_CRYPTO_GCM=y
    CONFIG_RFKILL=y
    CONFIG_REGULATOR_FIXED_VOLTAGE=y
    CONFIG_CRC7=y
    CONFIG_INPUT_UINPUT=y
)

ip_table_config=(
    CONFIG_NF_CONNTRACK=y
    CONFIG_NF_CONNTRACK_IPV4=y
    CONFIG_IP_NF_IPTABLES=y
    CONFIG_IP_NF_FILTER=y
    CONFIG_NF_NAT_IPV4=y
    CONFIG_IP_NF_TARGET_MASQUERADE=y
)

no_dt_config=(
    CONFIG_WL12XX_PLATFORM_DATA=y
)

bt_hci_config=(
    CONFIG_ST_HCI=y
)
function verify_configuration()
{
    conf_list=("${!1}")
    echo ""
    echo "Validating kernel .config ($1) "
    echo ""
    
    i="0"
	while [ $i -lt ${#conf_list[@]} ]; do
        cat $kernel_config_file 2> /dev/null | grep ${conf_list[i]} > /dev/null 2>&1        
        STATUS=$?
        if [ $STATUS  -eq 1 ] ; then
            echo "Missing - ${conf_list[i]}" 
            read -p "Do you want to add it [y/n] ? " yn
            case $yn in
                [Yy]* ) echo "${conf_list[i]}" >> ${kernel_config_file} && echo "${conf_list[i]} - Was Added!" && echo "";;
                [Nn]* ) echo "${conf_list[i]} was not added.";;
                * ) echo "Please answer y or n.";;
            esac   
        fi       
        i=$[$i + 1]
    done
    #echo "$1 scan completed"
}

if [ $# -lt 1 ]
then
	usage
fi
kernel_config_file=$1

if ! [ -f $kernel_config_file ];
then
   echo "Configuration file $kernel_config_file does not exists" 
   exit 1
fi

verify_configuration base_config[@]
[ $IP_TABLES ] && verify_configuration ip_table_config[@]
[ $NO_DEVICE_TREE ] && verify_configuration no_dt_config[@]
[ $BT_HCI ] && verify_configuration hci_config[@]
exit
