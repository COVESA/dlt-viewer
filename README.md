[![PullRequest](https://github.com/COVESA/dlt-viewer/actions/workflows/BuildPR.yml/badge.svg)](https://github.com/COVESA/dlt-viewer/actions/workflows/BuildPR.yml)

# Diagnostic Log and Trace Viewer

## Overview

The Diagnostic Log and Trace Viewer is an application that can send and receive control messages to the DLT daemon, e.g. to set individual log levels of applications and contexts or get the list of applications and contexts registered in the DLT daemon. DLT viewer is based on Qt for Windows and Linux and can run on the Mac though that is not officially supported.

## Documentation

+ DLT Release Notes: ReleaseNotes.txt
+ DLT Installation: INSTALL.md

### Command line usage DLT Viewer:

+ Windows: `dlt-viewer.exe [OPTIONS] [logfile] [projectfile] [filterfile] [pcapfile] [mf4file]`
+ Linux: `dlt-viewer [OPTIONS] [logfile] [projectfile] [filterfile] [pcapfile] [mf4file]`
+ MacOS: `open -a $DLT_BUILD_DIR/bin/DLT\ Viewer.app --args [OPTIONS] [logfile] [projectfile] [filterfile] [pcapfile] [mf4file]`

Options:
  -c <textfile>                           Convert logfile file to <textfile>
  -u                                      Conversion will be done in UTF8
                                          instead of ASCII
  --csv                                   Conversion will be done in CSV format
  -d                                      Conversion will NOT be done, save in
                                          dlt file format again instead
  --dd                                    Conversion will NOT be done, save as
                                          decoded messages in dlt format
  -b <plugin|command|param1|..|param<n>>  Execute a plugin command with <n>
                                          parameters before loading log file.
  -e <plugin|command|param1|..|param<n>>  Execute a plugin command with <n>
                                          parameters after loading log file.
  -s, --silent                            Enable silent mode without any GUI.
                                          Ideal for commandline usage.
  --stream                                Treat the input logfiles as DLT
                                          stream instead of DLT files.
  -t, --terminate                         Terminate DLT Viewer after command
                                          line execution.
  -w <workingdirectory>                   Set the working directory
  --delimiter <character>                 The used delimiter for CSV export
                                          (Default: ,).
  -h, --help                              Print this help message.
  -v, --version                           Print the version.

Arguments:
  logfile                                 Loading one or more logfiles on
                                          startup (must end with .dlt)
  projectfile                             Loading project file on startup (must
                                          end with .dlp)
  filterfile                              Loading filterfile on startup (must
                                          end with .dlf)
  pcapfile                                Importing DLT/IPC from pcap file on
                                          startup (must end with .pcap)
  mf4file                                 Importing DLT/IPC from mf4 file on
                                          startup (must end with .mf4)


Examples:
  dlt-viewer.exe -t -c output.txt input.dlt
  dlt-viewer.exe -t -s -u -c output.txt input.dlt
  dlt-viewer.exe -t -s -d -c output.dlt input.dlt
  dlt-viewer.exe -t -s decoded.dlp -dd -c output.dlt input.dlt 
  dlt-viewer.exe -t -s -csv -c output.csv input.dlt
  dlt-viewer.exe -t -s -d filter.dlf -c output.dlt input.dlt
  dlt-viewer.exe -p export.dlp -e "Filetransfer Plugin|export|ftransferdir" input.dlt
  dlt-viewer.exe input1.dlt input2.dlt
  dlt-viewer.exe -t -c output.txt input.pcap
  dlt-viewer.exe -t -c output.txt input1.mf4 input2.mf4

### Command line usage DLT Commander:

Usage:

 dlt-commander [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]

Options:

 [logfile]	Loading one or more logfiles on startup (must end with .dlt)
 [filterfile]	Loading filterfile on startup (must end with .dlf)
 [pcapfile]	Importing DLT/IPC from pcap file on startup (must end with .pcap)
 [mf4file]	Importing DLT/IPC from mf4 file on startup (must end with .mf4)
 -h 	 Print usage
 -v or --version	Only show version and buildtime information
 -c textfile	Convert logfile file to textfile (logfile must end with .dlt)
 -u	Conversion will be done in UTF8 instead of ASCII
 -csv	Conversion will be done in CSV format
 -d	Conversion will NOT be done, save in dlt file format again instead
 -delimiter <character>	The used delimiter for CSV export (Default: ,).
 -multifilter	Multifilter will generate a separate export file with the name of the filter.
             	-c will define the folder name, not the filename.

Examples:

 dlt-commander -c .\trace.txt c:\trace\trace.dlt
 dlt-commander -c -u .\trace.txt c:\trace\trace.dlt
 dlt-commander -d -c .\trace.dlt c:\trace\trace.dlt
 dlt-commander -csv -c .\trace.csv c:\trace\trace.dlt
 dlt-commander -d -c .\filteredtrace.dlt c:\filter\filter.dlf c:\trace\trace.dlt
 dlt-commander trace_1.dlt trace_2.dlt
 dlt-commander input.pcap output.dlt
 dlt-commander -c output.txt input.pcap
 dlt-commander -c output.txt input1.mf4 input2.mf4


## API Documentation

See INSTALL.txt regarding doxygen API documentation generation.

## Software/Hardware

Developed with QT5 and Qt6 (http://qt-project.org/)
Tested with Windows 11 + Qt 5.15.2 + Microsoft Visual Studio Build Tools 2019
Tested with Windows 11 + Qt 6.7.3 + Microsoft Visual Studio Build Tools 2019
Tested with Ubuntu Linux 24.04 + Qt 5.15.13
Tested with macOS Catalina + Qt 5.15.13
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

## Apple AppStore

https://apps.apple.com/us/app/dlt-viewer/id6648796931?mt=12

## Contact

Alexander Wenzel Alexander.AW.Wenzel@bmw.de and Gernot Wirschal Gernot.Wirschal@bmw.de
