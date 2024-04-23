#!/bin/bash
# building DLT Viewer debian packages on Ubuntu 20.04 / Focal Fossa
# script created 19.6.2018, Gernot Wirschal
# updated 23.04.2024, Alexander Wenzel
set -e
rm -rf debtmp
DTLREV=`git rev-list --all --count`
cp -r focal/debian .
sed -i '/#define PACKAGE_REVISION/c #define PACKAGE_REVISION "'$DTLREV'"' src/version.h
DLT_VERSION=`cat src/version.h | grep "PACKAGE_VERSION" | grep -v PACKAGE_VERSION_STATE | awk '{print $3}' | tr -d \" `
sed -i '/covesa-dlt-viewer/c covesa-dlt-viewer ('${DLT_VERSION}') stable; urgency=low' ./debian/changelog
echo "DLT_VERSION: ${DLT_VERSION}, $DTLREV"

mkdir -p ./debtmp/dlt-viewer-${DLT_VERSION}
rsync -a --exclude=debtmp --exclude=*.sh --exclude=*.bat --exclude=cache --exclude=parser --exclude=bionic * debtmp/dlt-viewer-${DLT_VERSION}

cd debtmp/dlt-viewer-${DLT_VERSION}
echo "#############################################"
echo "Create tarball ../covesa-dlt-viewer_${DLT_VERSION}.orig.tar.gz"
echo "#############################################"

tar -czf ../covesa-dlt-viewer_${DLT_VERSION}.orig.tar.gz *
echo "y" | debuild -uc -us
#make -j8 install

