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


scp -i ~/Downloads/velo.pem ../linux-*.deb  admin@apt.twonav.com:/home/admin/Downloads/Experimental
ssh -i ~/Downloads/velo.pem admin@apt.twonav.com "/home/admin/update-repository.sh linux-*.deb Experimental Kernel"
