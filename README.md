[![PullRequest](https://github.com/COVESA/dlt-viewer/actions/workflows/BuildPR.yml/badge.svg)](https://github.com/COVESA/dlt-viewer/actions/workflows/BuildPR.yml)

# Diagnostic Log and Trace Viewer

## Overview

The Diagnostic Log and Trace Viewer is an application that can send and receive control messages to the DLT daemon, e.g. to set individual log levels of applications and contexts or get the list of applications and contexts registered in the DLT daemon. DLT viewer is based on Qt for Windows and Linux and can run on the Mac though that is not officially supported.

## Documentation

+ DLT Release Notes: ReleaseNotes.txt
+ DLT Installation: INSTALL.md

### Command line usage;

+ Windows: `dlt-viewer.exe [OPTIONS] [logfile] [projectfile] [filterfile]`
+ Linux: `dlt-viewer [OPTIONS] [logfile] [projectfile] [filterfile]`
+ MacOS: `open -a $DLT_BUILD_DIR/bin/DLT\ Viewer.app --args [OPTIONS] [logfile] [projectfile] [filterfile]`

Options: 
+ [logfile] Loading one or more logfiles on startup (must end with .dlt)
+ [projectfile] Loading project file on startup (must end with .dlp)
+ [filterfile] Loading filterfile on startup (must end with .dlf)
+ `-h` Print usage
+ `-c textfile`  Convert logfile file to textfile
+ `-u` Conversion will be done in UTF8 instead of ASCII;
+ `-csv` Conversion will be done in CSV format;
+ `-d` Conversion will NOT be done, save in dlt file format again instead
+ `-dd` Conversion will NOT be done, save as decoded messages in dlt format
+ `-b "name of plugin|command in plugin|param1|..|param<n>"` Execute a plugin command with <n> parameters before loading log file
+ `-e "name of plugin|command in plugin|param1|..|param<n>"` Execute a plugin command with <n> parameters after loading log file
+ `-s or --silent`  Enable silent mode without warning message boxes
+ `-t or --terminate` Terminate DLT Viewer after command line execution
+ `-v or --version` Only show version and buildtime information
+ `-w workingdirectory` Set the working directory

## API Documentation

See INSTALL.txt regarding doxygen API documentation generation.

## Software/Hardware

Developed with QT5 and Qt6 (http://qt-project.org/)
Tested with Windows 11 + Qt 5.15.2 + Microsoft Visual Studio Build Tools 2019
Tested with Windows 11 + Qt 6.7.1 + Microsoft Visual Studio Build Tools 2019
Tested with Ubuntu Linux 18.04 + Qt 5.9.5
Tested with macOS Sierra + Qt 5.7.0
Dynamically linked open source software

Qt 5.x.x (LGPLv3 - Qt)
GCC (GPL)
(Optional) Mingw (GPL + BSD Variante + Public Domain)
Open Icon Library (http://openiconlibrary.sourceforge.net,Multiple Licenses)
License

Full information on the license for this software is available in the "LICENSE.txt" file.

## Source Code for DLT Viewer (This repository)

https://github.com/COVESA/dlt-viewer 

## Homepage

[Diagnostic Log and Trace Homepage](https://github.com/COVESA/dlt-viewer) on Github

## Issues and questions

[Diagnostic Log and Trace Homepage Issues](https://github.com/COVESA/dlt-viewer/issues)

## Additional Plugins

https://github.com/svlad-90/DLT-Message-Analyzer

## Contact

Alexander Wenzel Alexander.AW.Wenzel@bmw.de and Gernot Wirschal Gernot.Wirschal@bmw.de
