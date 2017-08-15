Diagnostic Log and Trace - Readme
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
 -h                     Print usage (only in Linux)
 -p projectfile         Loading project file on startup (must end with .dlp)
 -l logfile             Loading logfile on startup (must end with .dlt)
 -f filterfile          Loading filterfile on startup (must end with .dlf)
 -c logfile textfile    Convert logfile file to textfile (logfile must end with .dlt)
 -e "name of plugin|command in plugin|param1|param2|paramn..."
                        Execute command in a command interface plugin.
 -s                     Enable silent mode without warning message boxes


API Documentation
-----------------
See INSTALL.txt regarding doxygen API documentation generation.

Software/Hardware
-----------------

* Developed with QT 5 (http://qt-project.org/)
* Tested with Windows 7 + Qt 5.5.1 + MinGW 4.9.1
* Tested with Windows 7 + Qt 5.5.1 + MSVC 2013 Express
* Tested with Ubuntu Linux 14.04 + Qt 5.5.1

Dynamically linked open source software
---------------------------------------

* Qt 5.5.1 (LGPLv3 - Qt)
* GCC (GPL)
* qextserialport (MIT License - Version 1.2 BETA)
* (Optional) Mingw (GPL + BSD Variante + Public Domain)
* Open Icon Library (http://openiconlibrary.sourceforge.net,Multiple LIcenses)

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
Lutz Helwind <Lutz_Helwing@mentor.com>

