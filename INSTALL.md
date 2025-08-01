# DLT Viewer - Installation

Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>

## Instructions for installing DLT Viewer (GUI)

* Install Qt5 SDK and Qt6 SDK including Qt Creator and Microsoft Visual Studio Build Tools 2019
* Tested with QT 5.15.2 (Windows), QT 6.7.3 (Windows), and Qt 5.15.13 (Ubuntu 24.04LTS Linux)
* Open project CMakeLists.txt in Qt Creator.
* Ensure that build options point to correct QT and MSVC.
* Build Release in Qt Creator or via CMake (see last).
* Optional Linux: set the library path in Qt Creator: Add variable to Projects/Build Settings/Build Environment: LD_LIBRARY_PATH = .

## Instructions for installing DLT Viewer (Linux command line)

* sudo apt install build-essential
* sudo apt install qtcreator
* sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
* sudo apt install libqt5serialport5-dev
* mkdir build
* cd build
* cmake ..
* make
* sudo make install
* sudo ldconfig
* Optional: set the library path: LD_LIBRARY_PATH = .

## Instructions for installing DLT Viewer (macOS)

* Download and install DLTViewer from the Mac AppStore
* https://apps.apple.com/us/app/dlt-viewer/id6648796931?mt=12

## Build DLT Viewer in Windows

* Install Qt5 SDK or Qt6 SDK and Microsoft Visual Studio Build Tools 2019
* If needed adapt you configuration in build_config.bat
* Run build_sdk_windows_qt5_MSVC_interactive.bat

The DLT Viewer will be installed by default in C:\DltViewerSDK

## Create SDK (Linux)

* Compile DLT Viewer as described in "Build DLT-viewer Linux"
* Call "sudo make install" to install dlt-viewer including libraries and headers for SDK

## Create SDK Documentation (Windows)

* Install doxygen and graphviz
* Change into project directory
* doxygen sdk\doxygen_dlt_viewer_plugininterface.cfg
* (Optional) doxygen sdk\doxygen_dlt_viewer.cfg
* (Optional) doxygen sdk\doxygen_dlt_viewer_qdlt.cfg
* You will find the documentation in the doc directory

## Create SDK Documentation (Linux)

* Install doxygen and graphviz
* Change into project directory
* doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
* (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
* (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
* You will find the documentation in the doc directory

## Create manuals documentation

The plugins programming manual is generated with asciidoc.
Call 
    asciidoc dlt_viewer_user_manual.txt
    asciidoc dlt_viewer_plugins_programming_guide.txt
to generate html output.
To generate pdfs, call
    sh convert.sh
from within the doc directory

The user manual is of Latex input format since June 2018
Build verified on an Ubuntu 18.04 installation with
sudo apt-get install texlive texlive-latex-extra

To create a pdf format output use:
pdflatex dlt-viewer_user_manual.tex

## Instructions for building DLT Viewer with CMake (Linux and MacOS command line)

CMake is a "meta" make system.  This means CMake creates a make system 
for various make solutions including command line build tools like
"Unix Makefiles", or IDE tools like "Visual Studio".  
See:  https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html

CMake encourages the "out of source" build paradigm. Assuming DLT Viewer 
code has been unpacked in src_path/, we encourage not building in that directory

* mkdir build
* cd build
* cmake "GENERATOR OF CHOICE" src_path/

At this stage, CMake will have produced a set of filesthat can be used by the
selected make system, be it "Unix Makefiles", "Visual Studio 6" or  other.
Often a DLT-Viewer.EXTENSION file will be present in the build directory.

In the case of "Unix Makefiles" building is now as simple as

* sudo make install
* sudo ldconfig
* Optional: set the library path: LD_LIBRARY_PATH = .

Various solutions exist for cmake configuration to be informed about 
non-standard installation paths for required components.  See CMake
documentation for details. (hint: QTDIR)

For macOS, you can install Qt with Homebrew:
brew install qt@5

Then give the Qt directory to CMake and build:
#### x86:
* Qt5_DIR="/usr/local/opt/qt" cmake src_path/
* make
#### arm64:
* Qt5_DIR="/opt/homebrew/opt/qt5" cmake src_path/
* make

The application is built in "bin/DLT Viewer.app", it can be launched from Finder or the command line:
open -a $DLT_BUILD_DIR/bin/DLT\ Viewer.app

## Building DLT Viewer release version with Qmake on macOS

Sometimes building with Cmake or in Qt Creator leads to Plugins not working in  DLT Viewer.
Building with qmake in release version gets rid of this problem.
Steps to follow:

* mkdir build
* cd build
* TODO: Change to cmake
* <path to Qt folder>/Qt/5.X/gcc_64/bin/qmake <path to BuildDltViewer.pro>/BuildDltViewer.pro -r
* make

At this point "release" folder should be created in the "build" folder

* cd release
* export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

To run:

*./dlt_viewer
