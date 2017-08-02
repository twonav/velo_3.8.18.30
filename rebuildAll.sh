if [ $# -ne 1 ] ; then
echo "Usage: ./rebuildAll.sh <VERSION>"
exit
fi

VERSION=$1

./build_backports.sh base $VERSION
./build_backports.sh twonav_velo $VERSION
./build_backports.sh twonav_aventura $VERSION
./build_backports.sh twonav_horizon $VERSION
./build_backports.sh twonav_trail $VERSION
./build_backports.sh os_velo $VERSION
./build_backports.sh os_aventura $VERSION
./build_backports.sh os_horizon $VERSION
./build_backports.sh os_trail $VERSION

