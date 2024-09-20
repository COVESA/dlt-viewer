#!/bin/bash
# building DLT Viewer debian packages on Ubuntu 24.04 / Noble
# script created 19.6.2018, Gernot Wirschal
# updated 11.06.2024, Alexander Wenzel
set -e
rm -rf debtmp
DTLREV=`git rev-list --all --count`
DEBIAN_REVISION=${DTLREV}
PACKAGE_NAME=covesa-dlt-viewer
cp -r noble/debian .
sed -i '/#define PACKAGE_REVISION/c #define PACKAGE_REVISION "'$DTLREV'"' src/version.h
DLT_VERSION=`cat src/version.h | grep "PACKAGE_VERSION" | grep -v PACKAGE_VERSION_STATE | awk '{print $3}' | tr -d \" `
DLT_VERSION_STATE=`cat src/version.h | grep "PACKAGE_VERSION_STATE" | awk '{print $3}' | tr -d \" `
sed -i '/covesa-dlt-viewer/c covesa-dlt-viewer ('${DLT_VERSION}-${DEBIAN_REVISION}') '${DLT_VERSION_STATE}'; urgency=low' ./debian/changelog
sed -i '$ d' ./debian/changelog
date=$(date -Ru)
echo " -- Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>  $date" >> ./debian/changelog

echo "DLT_VERSION: ${DLT_VERSION}"
echo "DEBIAN_REVISION: ${DEBIAN_REVISION}"
echo "DLT_VERSION_STATE: ${DLT_VERSION_STATE}"
echo "PACKAGE_NAME: ${PACKAGE_NAME}"

mkdir -p ./debtmp/${PACKAGE_NAME}-${DLT_VERSION}-${DEBIAN_REVISION}
rsync -a --exclude=debtmp --exclude=*.sh --exclude=*.bat --exclude=cache --exclude=parser --exclude=bionic * debtmp/${PACKAGE_NAME}-${DLT_VERSION}

cd debtmp/${PACKAGE_NAME}-${DLT_VERSION}
echo "#############################################"
echo "Create tarball ../${PACKAGE_NAME}_${DLT_VERSION}-${DEBIAN_REVISION}.orig.tar.gz"
echo "#############################################"

tar -czf ../${PACKAGE_NAME}_${DLT_VERSION}-${DEBIAN_REVISION}.orig.tar.gz *
echo "y" | debuild -uc -us
#make -j8 install

