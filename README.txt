DLT Viewer - Automotive Diagnostic Log and Trace Viewer

Version 2.5.0_BETA

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


Dynamically linked/open source software
--------------------------------------
* Qt 4.7.4 (LGPL - Qt SDK version 1.2)
* GCC (GPL)
* qextserialport (New BSD licence - Version 1.1)
* (Optional) Mingw (GPL + BSD Variante + Public Domain)


Compiling in Windows:
---------------------
- Download and install "Qt SDK version 1.2" for Windows from http://qt.nokia.com/downloads
- Download qextserialport (REL_1_1) from http://code.google.com/p/qextserialport/
- Save file <DLT-viewer-Root>/qextserialport/qextserialport.pro
- Copy qextserialport (REL_1_1) into <DLT-viewer-Root>/qextserialport directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Open project BuildDltViewer.pro in Qt Creator
- Build Release in Qt Creator


Compiling in Linux:
-------------------
Precondition
- Install package Qt4 including developer tools
- Optional: Install Qt Creator

Steps
- Download qextserialport (REL_1_1) from http://code.google.com/p/qextserialport/
- Save file qextserialport/qextserialport.pro
- Copy qextserialport (REL_1_1) into <DLT-viewer-Root>/qextserialport directory 
- Overwrite file qextserialport/qextserialport.pro with saved file before
- Create directory build
- Change into directory build
- qmake-qt4 ../BuildDltViewer.pro
- make
- optional: sudo make install
- optional: sudo ldconfig
- Run dlt_viewer (if you installed the viewer) or change into the release directory and run ./dlt_viewer 
- Optional: Open project BuildDltViewer.pro in Qt Creator and build DLT Viewer in Qt Creator


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



Create Linux SDK:
-----------------
- Compile DLT Viewer as described in "Compiling in Linux"
- Call "sudo make install" to install dlt_viewer including libraries and headers for SDK


Create SDK Documentation for Linux and Windows:
-----------------------------------------------
- Install doxygen
- Change into project directory
- doxygen sdk/doxygen_dlt_viewer_plugininterface.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer.cfg
- (Optional) doxygen sdk/doxygen_dlt_viewer_qdlt.cfg
- You will find the documentation in the doc directory
