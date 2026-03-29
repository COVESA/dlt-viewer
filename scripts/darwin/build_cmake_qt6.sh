#!/usr/bin/env bash
set -ex

SRC_DIR=$(pwd)
BUILD_DIR="${SRC_DIR}/build"
INSTALL_DIR="${BUILD_DIR}/install"
APP_DIR_NAME="DLTViewer.app"

rm -rf "${INSTALL_DIR}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "QT_ROOT_DIR=$QT_ROOT_DIR"

echo Cleanup
rm -rf "${INSTALL_DIR}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo 'export PATH="/Users/runner/work/dlt-viewer/Qt/6.8.3/macos:$PATH"' >> ~/.bash_profile

echo Build with CMake
# Installation paths configuration creates proper macOS Application bundle structure
# https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html
cmake -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
  -DCMAKE_PREFIX_PATH=${QT_ROOT_DIR}/lib/cmake \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
  -DCMAKE_BUILD_TYPE=Release \
  -DDLT_USE_QT_RPATH=ON \
  -DDLT_PARSER=OFF \
  -DDLT_APP_DIR_NAME=${APP_DIR_NAME} \
  -DDLT_LIBRARY_INSTALLATION_PATH="${APP_DIR_NAME}/Contents/Frameworks" \
  -DDLT_EXECUTABLE_INSTALLATION_PATH="${APP_DIR_NAME}/Contents/MacOS" \
  -DDLT_RESOURCE_INSTALLATION_PATH="${APP_DIR_NAME}/Contents/Resources" \
  -DDLT_PLUGIN_INSTALLATION_PATH="${APP_DIR_NAME}/Contents/MacOS/plugins" \
  "${SRC_DIR}"
cmake --build "${BUILD_DIR}" -j$(sysctl -n hw.ncpu)

cd ${SRC_DIR}

cmake --install build --prefix "${INSTALL_DIR}"
ls -l build/install/DLTViewer.app
otool -L build/install/DLTViewer.app/Contents/MacOS/dlt-viewer
otool -l build/install/DLTViewer.app/Contents/MacOS/dlt-viewer | grep -A2 LC_RPATH
echo "Deploying DLTViewer.app with macdeployqt"
$QT_ROOT_DIR/bin/macdeployqt build/install/DLTViewer.app -verbose=2 -dmg \
  -libpath=$(pwd)/build/install/DLTViewer.app/Contents/Frameworks \
  -executable=$(pwd)/build/install/DLTViewer.app/Contents/MacOS/dlt-viewer


cd "${BUILD_DIR}"
FULL_VERSION=$(cat "${BUILD_DIR}/full_version.txt")
echo "FULL_VERSION=${FULL_VERSION}"

mkdir -p dist
cp ../scripts/darwin/install.md dist
