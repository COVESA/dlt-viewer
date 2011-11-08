DLT Viewer - Automotive Diagnostic Log and Trace Viewer

Version 2.4.1

The DLT Viewer can send control messages to the daemon, e.g. to set 
individual log levels of applications and contexts or get the list of 
applications and contexts registered in the DLT daemon.


Homepage
--------
https://collab.genivi.org/wiki/display/geniviproj/Automotive+DLT+%28Diagnostic+Log+and+Trace%29


License
-------
Full information on the license for this software
is available in the "LICENSE.txt" file. 
The full LGPL license is in "LGPL.txt."


Contact
-------
Alexander Wenzel (Alexander.AW.Wenzel@bmw.de)
Christian Muck (christian.muck@bmw.de)


Dynamically linked open source software
-----------------------------------
* Qt 4.7 (LGPL - Qt SDK version 1.1.3)
* GCC (GPL)
* Mingw (GPL + BSD Variante + Public Domain)
* qextserialport (New BSD licence - Version 1.1)


Compiling in Windows:
---------------------
- Download "Qt SDK version 1.1.3" for Windows from http://qt.nokia.com/downloads
- Download qextserialport V1.1 from http://sourceforge.net/projects/qextserialport/files/qextserialport
- Install "Qt SDK version 1.1.3" (use custom install, normally symbian stuff and emulators not needed, select MinGW if needed)
- Save file qextserialport/qextserialport.pro
- Extract qextserialport package into DLT-viewer main directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Open project BuildDltViewer.pro in Qt Creator
- Build Release in Qt Creator


Compiling in Linux:
-------------------
- Install package Qt4 including developer tools
- Optional: Install Qt Creator
- Download qextserialport V1.1 from http://sourceforge.net/projects/qextserialport/
- Save file qextserialport/qextserialport.pro
- Extract qextserialport package into DLT-viewer main directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Create directory build
- Change into directory build
- qmake-qt4 ../BuildDltViewer.pro
- make
- optional: sudo make install
- optional: sudo ldconfig
- Run dlt_viewer (if you installed the viewer) or change into the release directory and run ./dlt_viewer 
- optional: Open project BuildDltViewer.pro in Qt Creator and build DLT Viewer in Qt Creator


Command line usage:
-------------------
Usage windows: dlt_viewer.exe [OPTIONS] 
Usage Linux: dlt_viewer [OPTIONS] 
Options: 
 -h 			Print usage (only in Linux)
 -p projectfile 	Loading project file on startup (must end with .dlp) 
 -l logfile 		Loading logfile on startup (must end with .dlt) 
 -f filterfile 		Loading filterfile on startup (must end with .dlf) 
 -c logfile textfile 	Convert logfile file to textfile (logfile must end with .dlt) 


Create Windows SDK:
-------------------
- Compile DLT Viewer as described in "Compiling in Windows"
- Modify pathes in sdk\create_sdk.bat to your needs
- Execute sdk\create_sdk.bat batch file


Create Windows SDK Documentation:
-------------------------------
- Install doxygen
- Change into project directory
- doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
- You will find the documentation in the doc directory


Create Linux SDK:
-----------------
- Compile DLT Viewer as described in "Compiling in Linux"
- Call "sudo make install" to install dlt_viewer including libraries and headers for SDK


Create Linux SDK Documentation:
-------------------------------
- Install doxygen and graphviz
- Change into project directory
- doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
- You will find the documentation in the doc directory

