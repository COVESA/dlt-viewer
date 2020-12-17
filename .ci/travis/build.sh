#!/bin/bash

if [[ "$(uname -s)" == 'Darwin' ]]; then
  readonly Qt5_DIR="/usr/local/opt/qt"
fi

mkdir build
cd build

# Building with CMake
cmake ../
make

# Cleanup
rm -rf *

# Building with QMake
qmake ../BuildDltViewer.pro
make
