#!/usr/bin/env bash
set -ex

# https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.12.10-bionic
source /opt/qt512/bin/qt512-env.sh || true

SRC_DIR=$(pwd)
BUILD_DIR="${SRC_DIR}/build"
INSTALL_DIR="${BUILD_DIR}/install"
APP_DIR_NAME="DLTViewer"

rm -rf "${APP_DIR_NAME}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo Build with QMake
qmake ../BuildDltViewer.pro
make

echo Cleanup
rm -rf "${INSTALL_DIR}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo Build with CMake
cmake -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
  -DCMAKE_PREFIX_PATH=/opt/qt512/lib/cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DDLT_USE_QT_RPATH=ON \
  -DDLT_PARSER=OFF \
  -DDLT_APP_DIR_NAME=${APP_DIR_NAME} \
  -DDLT_LIBRARY_INSTALLATION_PATH="${APP_DIR_NAME}/usr/lib" \
  -DDLT_EXECUTABLE_INSTALLATION_PATH="${APP_DIR_NAME}/usr/bin" \
  -DDLT_RESOURCE_INSTALLATION_PATH="${APP_DIR_NAME}/usr/share" \
  -DDLT_PLUGIN_INSTALLATION_PATH="${APP_DIR_NAME}/usr/bin/plugins" \
  "${SRC_DIR}"
cmake --build "${BUILD_DIR}" -v

# External CPack generator calls "cmake --install" and "linuxdeploy"
#
# CMake install takes care of proper AppDir setup. Each CMake target has a pre-configured path.
# - https://github.com/linuxdeploy/linuxdeploy
# - https://github.com/AppImage/appimagekit
#
# linuxdeploy will establish relative DT_RUNPATH: $ORIGIN/../lib inside each binary file.
# https://docs.appimage.org/reference/best-practices.html#general-recommendations
# https://docs.appimage.org/packaging-guide/from-source/native-binaries.html
# DT_RUNPATH https://linux.die.net/man/1/ld
# could be checked with `objdump -p`

# sudo chown -R 1000:1000 "${SRC_DIR}/build"
# sudo chmod -R a+rw "${SRC_DIR}/build"

# linuxdeploy is wrapped in CPack External generator. See linux/package.cmake
# Please note, other CPack generators will NOT call linuxdeploy, therefore they will produce non-relocatable package.
cpack -G External

cd "${BUILD_DIR}"
FULL_VERSION=$(cat "${BUILD_DIR}/full_version.txt")
echo "FULL_VERSION=${FULL_VERSION}"

mkdir -p dist
cp ../scripts/linux/install.md dist
tar -czvf "dist/DLTViewer-${FULL_VERSION}.tgz" -C ${INSTALL_DIR} .
