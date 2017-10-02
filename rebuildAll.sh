if [ $# -ne 1 ] ; then
echo "Usage: ./rebuildAll.sh <VERSION>"
exit
fi

VERSION=$1

./build_backports.sh twonav $VERSION
./build_backports.sh flasher $VERSION
./build_backports.sh tester $VERSION


#scp -i ~/Downloads/velo.pem ../linux-*.deb  admin@apt.twonav.com:/home/admin/Downloads/Experimental
#ssh -i ~/Downloads/velo.pem admin@apt.twonav.com "/home/admin/update-repository.sh linux-*.deb Experimental Kernel"
