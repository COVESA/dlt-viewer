#!/usr/bin/env bash
set -ex

PLATFORM=$(uname -s)
if [[ "${PLATFORM}" == "Darwin" ]]; then
  readonly Qt5_DIR="/usr/local/opt/qt"
elif [[ "${PLATFORM}" == "Linux" ]]; then
  # https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.12.10-bionic
  source /opt/qt512/bin/qt512-env.sh || true
fi

SRC_DIR=$(pwd)
BUILD_DIR="${SRC_DIR}/build"
APP_DIR="${BUILD_DIR}/DLTViewer"

rm -rf "${APP_DIR}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo Building with QMake
qmake ../BuildDltViewer.pro
make

echo Cleanup
rm -rf "${APP_DIR}"
rm -rf "${SRC_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo Building with CMake

cmake -G Ninja \
  -DCMAKE_INSTALL_PREFIX=DLTViewer \
  -DCMAKE_PREFIX_PATH=/opt/qt512/lib/cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DDLT_USE_QT_RPATH=ON \
  -DDLT_PARSER=OFF \
  -DDLT_LIBRARY_INSTALLATION_PATH="usr/lib" \
  -DDLT_EXECUTABLE_INSTALLATION_PATH="usr/bin" \
  -DDLT_RESOURCE_INSTALLATION_PATH="usr/share" \
  -DDLT_PLUGIN_INSTALLATION_PATH="usr/bin/plugins" \
  "${SRC_DIR}"
cmake --build "${BUILD_DIR}" -v

# CMake install takes care of proper AppDir setup. Each CMake target has a pre-configured path.
cmake --install "${BUILD_DIR}" --prefix "${APP_DIR}"

cd "${BUILD_DIR}"
mkdir -p dist

FULL_VERSION=$(cat "${BUILD_DIR}/full_version.txt")
echo "FULL_VERSION=${FULL_VERSION}"

if [[ "${PLATFORM}" == 'Darwin' ]]; then
  echo TODO Use macdeployqt https://doc.qt.io/qt-5/macos-deployment.html
  # AppDir archive
  tar -czvf "DLTViewer-${FULL_VERSION}.tgz" DLTViewer
  cp DLTViewer*.tgz dist
  cp ../scripts/darwin/install.md dist
elif [[ "${PLATFORM}" == "Linux" ]]; then
  cd "${BUILD_DIR}"
  # pwd
  # ls -la
  # https://github.com/linuxdeploy/linuxdeploy
  # https://github.com/AppImage/appimagekit

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

  mkdir -p dist
  cp DLTViewer*.AppImage dist
  cp ../scripts/linux/install.md dist
fi
