DLT Viewer - Installation
=========================
Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>

Instructions for installing DLT Viewer (GUI)
--------------------------------------------

* Install Qt5 SDK including Qt Creator and MSVC 2015 for 64bit Windows builds
* Tested with QT 5.12.4 (Windows) and Qt 5.9.5 (Ubuntu Linux)
* Open project BuildDltViewer.pro in Qt Creator.
* Ensure that build options point to correct QT and MSVC.
* Build Release in Qt Creator or via CMake (see last).
* Optional Linux: set the library path in Qt Creator: Add variable to Projects/Build Settings/Build Environment: LD_LIBRARY_PATH = .

Instructions for installing DLT Viewer (Linux command line)
----------------------------------------------------------

* sudo apt install build-essential
* sudo apt install qtcreator
* sudo apt install qt5-default
* sudo apt install libqt5serialport5-dev
* mkdir build
* cd build
* qmake ../BuildDltViewer.pro
* make
* sudo make install
* sudo ldconfig
* Optional: set the library path: LD_LIBRARY_PATH = .

Create SDK (Windows)
--------------------

Microsoft Visual C++ 2015 (64bit):
* Install Qt5 SDK and MSVC 2015 (you can use the free "Community" version).
* If your Qt installation does not match "C:\Qt\Qt5.12.4\5.12.4\msvc2015_64" (for 64 bit),
    you can set the correct path as environment variable "QTDIR" in your system properties.
* Adjust and run build_sdk_windows_qt5_MSVC.bat with optional parameter (the current system's architecture is used by default).

The SDK will be installed to C:\Users\%USERNAME%\DltViewerSDK\

Optional: Prepare Installation Qwt (Windows)
--------------------------------------------

Some DLT Viewer plugins will use the Qwt library for displaying graphs.
The Qwt library must be compiled and installed before it can be used.

(Tested with Qwt 6.1.4)

Microsoft Visual C++ 2015 (64bit):
* Install Qt5 SDK for Visual C++ 2015 (you can use the free "Community" version)
* Extract Qwt Zip package
* If you like, you can adjust the installation path by changing the variable "QWT_INSTALL_PREFIX" in qwtconfig.pri
* Start Qt commandline from Start->Qt_5.x.x
* Change into Qwt directory
* Execute "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
    for 64bit.
* Execute qmake qwt.pro
* Execute "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe"
* Execute "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe" install

Create SDK (Linux)
------------------

* Compile DLT Viewer as described in "Build DLT-viewer Linux"
* Call "sudo make install" to install dlt-viewer including libraries and headers for SDK

Create SDK Documentation (Windows)
----------------------------------

* Install doxygen and graphviz
* Change into project directory
* doxygen sdk\doxygen_dlt_viewer_plugininterface.cfg
* (Optional) doxygen sdk\doxygen_dlt_viewer.cfg
* (Optional) doxygen sdk\doxygen_dlt_viewer_qdlt.cfg
* You will find the documentation in the doc directory

Create SDK Documentation (Linux)
--------------------------------

* Install doxygen and graphviz
* Change into project directory
* doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
* (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
* (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
* You will find the documentation in the doc directory

Create manuals documentation
----------------------------

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



Instructions for building DLT Viewer with CMake (Linux and MacOS command line)
----------------------------------------------------------
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

For MacOS, you can install Qt with Homebrew:
brew install qt

Then give the Qt directory to CMake and build:
* Qt5_DIR="/usr/local/opt/qt" cmake src_path/
* make

The application is built in "bin/DLT Viewer.app", it can be launched from Finder or the command line:
open -a $DLT_BUILD_DIR/bin/DLT\ Viewer.app

Building DLT Viewer release version with Qmake on MacOs
--------------------------------------------------------
Sometimes building with Cmake or in Qt Creator leads to Plugins not working in  DLT Viewer.
Building with qmake in release version gets rid of this problem.
Steps to follow:

* cd dlt-viewer
* git clone https://github.com/GENIVI/dlt-viewer.git
* mkdir build
* cd build
* <path to Qt folder>/Qt/5.X/gcc_64/bin/qmake <path to BuildDltViewer.pro>/BuildDltViewer.pro -r
* make

At this point "release" folder should be created in the "build" folder

* cd release
* export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

To run:

*./dlt_viewer
