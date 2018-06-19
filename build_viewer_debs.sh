#!/bin/bash
# building DLT Viewer debian packages on Ubuntu 18.04 / Bionic Beaver
# script created 19.6.2018, Gernot Wirschal
set -e
rm -rf debtmp
DTLREV=`git rev-list --all --count`
cp -r bionic/debian .
sed -i '/#define PACKAGE_REVISION/c #define PACKAGE_REVISION "'$DTLREV'"' src/version.h
DLT_VERSION=`cat src/version.h | grep "PACKAGE_VERSION" | grep -v PACKAGE_VERSION_STATE | awk '{print $3}' | tr -d \" `
sed -i '/genivi-dlt-viewer/c genivi-dlt-viewer ('${DLT_VERSION}') stable; urgency=low' ./debian/changelog
echo "DLT_VERSION: ${DLT_VERSION}, $DTLREV"

mkdir -p ./debtmp/dlt-viewer-${DLT_VERSION}
rsync -a --exclude=debtmp --exclude=*.sh --exclude=*.bat --exclude=cache --exclude=parser --exclude=bionic * debtmp/dlt-viewer-${DLT_VERSION}

cd debtmp/dlt-viewer-${DLT_VERSION}
echo "#############################################"
echo "Create tarball ../genivi-dlt-viewer_${version}.orig.tar.gz"
echo "#############################################"

tar -czf ../genivi-dlt-viewer_${DLT_VERSION}.orig.tar.gz *
echo "y" | debuild -uc -us
#make -j8 install

