#!/usr/bin/env bash
set -ex

SRC_DIR=$(pwd)
BUILD_DIR="${SRC_DIR}/build"
INSTALL_DIR="${BUILD_DIR}/install"
APP_DIR_NAME="DLTViewer"

NPROC=$(nproc)
echo Nb of cpus: ${NPROC}

echo Cleanup
rm -rf "${APP_DIR_NAME}"
rm -rf "${INSTALL_DIR}"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo Build with CMake
cmake -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
  -DCMAKE_BUILD_TYPE=Release \
  -DDLT_USE_QT_RPATH=ON \
  -DDLT_PARSER=OFF \
  -DDLT_APP_DIR_NAME=${APP_DIR_NAME} \
  -DDLT_LIBRARY_INSTALLATION_PATH="${APP_DIR_NAME}/usr/lib" \
  -DDLT_EXECUTABLE_INSTALLATION_PATH="${APP_DIR_NAME}/usr/bin" \
  -DDLT_RESOURCE_INSTALLATION_PATH="${APP_DIR_NAME}/usr/share" \
  -DDLT_PLUGIN_INSTALLATION_PATH="${APP_DIR_NAME}/usr/bin/plugins" \
  "${SRC_DIR}"

cmake --build "${BUILD_DIR}" -j ${NPROC} -v
