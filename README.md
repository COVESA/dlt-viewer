# Diagnostic Log and Trace Viewer
## Overview

The Diagnostic Log and Trace Viewer is an application that can send and receive control messages to the DLT daemon, e.g. to set individual log levels of applications and contexts or get the list of applications and contexts registered in the DLT daemon. DLT viewer is based on Qt for Windows and Linux and can run on the Mac though that is not officially supported.

## Documentation

+ DLT Release Notes: ReleaseNotes.txt
+ DLT Installation: INSTALL.md

### Command line usage;

+ Windows: `dlt-viewer.exe [OPTIONS]`
+ Linux: `dlt-viewer [OPTIONS]`
+ MacOS: `open -a $DLT_BUILD_DIR/bin/DLT\ Viewer.app --args [OPTIONS]`

Options: 
+ `-h` Print usage
+ `-p projectfile` Loading project file on startup (must end with .dlp)
+ `-l logfile`     Loading logfile on startup (must end with .dlt)
+ `-f filterfile`  Loading filterfile on startup (must end with .dlf)
+ `-s or --silent`  Enable silent mode without warning message boxes
+ `-v or --version` Only show version and buildtime information
+ `-c logfile textfile`  Convert logfile file to textfile (logfile must end with .dlt)
+ `-u` Conversion will be done in UTF8 instead of ASCII";
+ `-csv` Conversion will be done in CSV format";
+ `-d` Conversion will NOT be done, save in dlt file format again instead"
+ `-dd` Conversion will NOT be done, save as decoded messages in dlt format"
+ `-e "name of plugin|command in plugin|param1|..|param<n>"` Execute a plugin command with <n> parameters

## API Documentation

See INSTALL.txt regarding doxygen API documentation generation.

## Software/Hardware

Developed with QT 5 (http://qt-project.org/)
Tested with Windows 10 + Qt 5.12.10 + Microsoft Visual Studio 2015 Community Edition
Tested with Ubuntu Linux 18.04 + Qt 5.9.5
Tested with macOS Sierra + Qt 5.7.0
Dynamically linked open source software

Qt 5.x.x (LGPLv3 - Qt)
GCC (GPL)
(Optional) Mingw (GPL + BSD Variante + Public Domain)
Open Icon Library (http://openiconlibrary.sourceforge.net,Multiple LIcenses)
License

Full information on the license for this software is available in the "LICENSE.txt" file.

## Source Code for DLT Viewer (This repository)

https://github.com/GENIVI/dlt-viewer

## Homepage

[Diagnostic Log and Trace](https://at.projects.genivi.org/wiki/display/PROJ/Diagnostic+Log+and+Trace) on GENIVI Projects Wiki

## Mailinglist

[genivi-diagnostic-log-and-trace](https://lists.genivi.org/mailman/listinfo/genivi-diagnostic-log-and-trace_lists.genivi.org)

## Additional Plugins

https://github.com/svlad-90/DLT-Message-Analyzer

## Contact

Alexander Wenzel Alexander.AW.Wenzel@bmw.de and Gernot Wirschal Gernot.Wirschal@bmw.de
