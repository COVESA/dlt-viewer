#!/usr/bin/env bash
set -ex

id
env
pwd

UBUNTU_VERSION=$(lsb_release -rs)
QT_PACKAGES=""
DEB_PACKAGES=""
# for ubuntu 22 install QT 5, for ubuntu 24 install QT 6
if [ "$UBUNTU_VERSION" = "22.04" ]; then
    QT_PACKAGES="qtbase5-dev libqt5serialport5-dev"
else
    QT_PACKAGES="qt6-base-dev libqt6serialport6-dev"
    DEB_PACKAGES="devscripts debhelper build-essential"
fi

sudo apt update
sudo apt-get install -y --no-install-recommends ninja-build ${QT_PACKAGES} ${DEB_PACKAGES} libgtest-dev

g++ --version
