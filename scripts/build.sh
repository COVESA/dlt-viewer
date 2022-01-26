#!/usr/bin/env bash
set -ex

PLATFORM=$(uname -s)
if [[ "${PLATFORM}" == "Darwin" ]]; then
  readonly Qt5_DIR="/usr/local/opt/qt"
elif [[ "${PLATFORM}" == "Linux" ]]; then
  # https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.12.10-bionic
  source /opt/qt512/bin/qt512-env.sh || true
fi

mkdir build
cd build

# Building with CMake
cmake -G Ninja \
    -DCMAKE_INSTALL_PREFIX=DLTViewer \
    -DCMAKE_PREFIX_PATH=/opt/qt512/lib/cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DDLT_USE_QT_RPATH=ON  \
    -DDLT_PARSER=OFF \
    -DDLT_LIBRARY_INSTALLATION_PATH="usr/lib" \
    -DDLT_EXECUTABLE_INSTALLATION_PATH="usr/bin" \
    -DDLT_RESOURCE_INSTALLATION_PATH="usr/share" \
    -DDLT_PLUGIN_INSTALLATION_PATH="usr/bin/plugins" ..
cmake --build . -v
cmake --install . --prefix "./DLTViewer"

# Cleanup
rm -rf *

# Building with QMake
qmake ../BuildDltViewer.pro
make
