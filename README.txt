License
-------
See file: LICENSE.txt
The full LGPL license: LGPL.txt

Dynamic linked open source software
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
- change into directory build
- qmake-qt4 ../BuildDltViewer.pro
- make
- optional: sudo make install
- optional: sudo ldconfig
- start dlt_viewer
- oprional: Open project BuildDltViewer.pro in Qt Creator and build DLT Viewer in Qt Creator
