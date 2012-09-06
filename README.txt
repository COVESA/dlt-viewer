DLT Viewer - Automotive Diagnostic Log and Trace Viewer

Version 2.7.0 ALPHA

The DLT Viewer can send control messages to the DLT daemon, e.g. to set 
individual log levels of applications and contexts or get the list of 
applications and contexts registered in the DLT daemon.
More features are listed on the Homepage.

Homepage
--------
https://collab.genivi.org/wiki/display/geniviproj/Automotive+DLT+%28Diagnostic+Log+and+Trace%29


License
-------
Full information on the license for this software
is available in the "LICENSE.txt" file. 
The full MPL license is in "MPL.txt."


Contact
-------
Alexander Wenzel (Alexander.AW.Wenzel@bmw.de)
Christian Muck (christian.muck@bmw.de)


Dynamically linked/open source software
--------------------------------------
* Qt 4.8.1 (LGPL - Qt SDK version 1.2)
* GCC (GPL)
* qextserialport (MIT license - Version 1.2 Beta)
* (Optional) Mingw (GPL + BSD Variante + Public Domain)


Compiling (Windows and Linux):
---------------------
- Install Qt4 (Qt SDK Version 1.2 - Guidline http://doc.qt.nokia.com/sdk-1.1/sdk-installing.html )
- Open project BuildDltViewer.pro in Qt Creator
- Build Release in Qt Creator
- (Optional in Linux) Change the directory to the build directory
- (Optional in Linux) sudo make install
- (Optional in Linux) sudo ldconfig
- (Optional in Linux) Optional set the library path in Qt Creator: Add variable to Projects/Build Settings/Build Environment: LD_LIBRARY_PATH = .|


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
