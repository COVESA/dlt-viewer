Diagnostic Log and Trace  - Readme
==================================
Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>

Overview
--------
The DLT Viewer can send control messages to the DLT daemon, e.g. to set 
individual log levels of applications and contexts or get the list of 
applications and contexts registered in the DLT daemon.
More features are listed on the Homepage.

Documentation
-------------
- DLT Release Notes: ReleaseNotes.txt
- DLT Installation: INSTALL.txt

Command line usage
------------------
Usage windows: dlt_viewer.exe [OPTIONS] 
Usage Linux: dlt_viewer [OPTIONS] 
Options: 
 -h 			Print usage (only in Linux)
 -p projectfile 	Loading project file on startup (must end with .dlp) 
 -l logfile 		Loading logfile on startup (must end with .dlt) 
 -f filterfile 		Loading filterfile on startup (must end with .dlf) 
 -c logfile textfile 	Convert logfile file to textfile (logfile must end with .dlt) 
 -e "name of plugin|command in plugin|param1|param2|paramn..."
    Execute command in a command interface plugin.

API Documentation
-----------------
See INSTALL.txt regarding doxygen API documentation generation.

Software/Hardware
-----------------

* Developped with: QT 4.8.1 (http://qt-project.org/)
* Ubuntu Linux 12.10 Qt SDK v1.2.1 32-bit / Intel PC
* MS Windows XP / Intel PC

Dynamically linked open source software
---------------------------------------

* Qt 4.8.1 (LGPL - Qt SDK version 1.2)
* GCC (GPL)
* qextserialport (MIT License - Version 1.2 BETA)
* (Optional) Mingw (GPL + BSD Variante + Public Domain)

License
-------
Full information on the license for this software is available in the "LICENSE.txt" file. 

Source Code
-----------
git://git.projects.genivi.org/dlt-viewer.git +
http://git.projects.genivi.org/dlt-viewer.git

Homepage
--------
http://projects.genivi.org/diagnostic-log-trace

Mailinglist
-----------
https://lists.genivi.org/mailman/listinfo/genivi-diagnostic-log-and-trace

Contact
-------
Alexander Wenzel <Alexander.AW.Wenzel@bmw.de> +
Christian Muck <christian.muck@bmw.de>

