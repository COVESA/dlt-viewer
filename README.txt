License
-------
Full information on the license for this software
is available in the "LICENSE.txt" file. 
The full LGPL license is in "LGPL.txt."

Dynamically linked open source software
-----------------------------------
* Qt 4.7 (LGPL)
* GCC (GPL)
* Mingw (GPL + BSD Variante + Public Domain)
* QextSerialPort (New BSD licence)

Compiling in Windows:
---------------------
- Download "Qt SDK for Windows" from http://qt.nokia.com/downloads
- Download qextserialport V1.1 from http://qextserialport.sourceforge.net/
- Install "Qt SDK for Windows"
- Save file qextserialport/qextserialport.pro
- Extract qextserialport package into DLT-viewer main directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Open project BuildDltViewer.pro in Qt Creator
- Build Release in Qt Creator

Compiling in Linux:
-------------------
- Install package Qt4 including developer tools
- Optional: Install Qt Creator
- Download qextserialport V1.1 from http://qextserialport.sourceforge.net/
- Save file qextserialport/qextserialport.pro
- Extract qextserialport package into DLT-viewer main directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Create directory build
- Change into directory build
- qmake-qt4 ../BuildDltViewer.pro
- make
- optional: sudo make install
- optional: sudo ldconfig
- start dlt_viewer
- optional: Open project BuildDltViewer.pro in Qt Creator and build DLT Viewer in Qt Creator

Create Windows SDK:
-------------------
- Compile DLT Viewer as described in "Compiling in Windows"
- Modify pathes in sdk\create_sdk.bat to your needs
- Execute sdk\create_sdk.bat batch file

Create Windows SDK Documentation:
-------------------------------
- Install doxygen
- Change into project directory
- doxygen sdk/doxygen.cfg
- You will find the documentation in the doc directory

Create Linux SDK:
-----------------
- Compile DLT Viewer as described in "Compiling in Linux"
- Call "sudo make install" to install dlt_viewer including libraries and headers for SDK

Create Linux SDK Documentation:
-------------------------------
- Install doxygen
- Change into project directory
- doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
- You will find the documentation in the doc directory

