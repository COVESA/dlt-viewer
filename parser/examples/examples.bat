echo off

echo ************************************
echo ***      DLT Parser Examples     ***
echo ************************************

echo .
echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

IF "%QTDIR%"=="" (
    set QTDIR=C:\Qt\Qt5.2.1\5.2.1\mingw48_32
)

IF "%MINGW_DIR%"=="" (
    set MINGW_DIR=C:\Qt\Qt5.2.1\Tools\mingw48_32
)

set PATH=%QTDIR%\bin;%MINGW_DIR%\bin;%PATH%

IF "%DLT_PARSER_EXE%"=="" (
        set DLT_PARSER_EXE=..\dlt-parser.exe
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * MINGW_DIR = %MINGW_DIR%
echo * PATH      = %PATH%
echo * DLT_PARSER_EXE = %DLT_PARSER_EXE%
echo ************************************

echo ************************************
echo ***  Delete old build Directory  ***
echo ************************************

rmdir /s/q result
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Prepare directories   ***
echo ************************************

mkdir result
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Command line options  ***
echo ************************************

%DLT_PARSER_EXE% --help
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Example 1             ***
echo ************************************
echo ** Normal Use Case                **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_1

%DLT_PARSER_EXE% --no-gui --parse-file example_1\mapp.c --update-id 1000 2000 --check-double --write-fibex result\example_1\mapp.xml --write-csv result\example_1\mapp.csv --write-id result\example_1
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Example 2             ***
echo ************************************
echo ** Error: Double ID Define        **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_2

%DLT_PARSER_EXE% --no-gui --parse-file example_2\mapp.c --update-id 1000 2000 --check-double --write-fibex result\example_2\mapp.xml --write-csv result\example_2\mapp.csv --write-id result\example_2

echo ************************************
echo ***        Example 3             ***
echo ************************************
echo ** Converte file                  **
echo ** - parse Files                  **
echo ** - converte Files               **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_3
copy example_3\capp.c result\example_3

%DLT_PARSER_EXE% --no-gui --parse-file result\example_3\capp.c --converte-file result\example_3\capp.c 
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

%DLT_PARSER_EXE% --no-gui --parse-file result\example_3\capp.c --update-id 1000 2000 --check-double --write-fibex result\example_3\capp.xml --write-csv result\example_3\capp.csv --write-id result\example_3
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Example 4             ***
echo ************************************
echo ** Parse Directory                **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_4

%DLT_PARSER_EXE% --no-gui --parse-dir example_4 --check-double-app --write-fibex result\example_4\fibex.xml --write-csv result\example_4\export.csv
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


echo ************************************
echo ***        Example 5             ***
echo ************************************
echo ***     Embedded DLT             ***
echo ************************************
echo ** Converte file                  **
echo ** Parse Directory                **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_5
copy example_5\* result\example_5

%DLT_PARSER_EXE% --no-gui --parse-file result\example_5\example5.cpp --converte-file result\example_5\example5.cpp 
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

%DLT_PARSER_EXE% --no-gui --parse-file result\example_5\example5.cpp --update-id 1000 2000 --check-double --write-fibex result\example_5\example5.xml --write-csv result\example_5\example5.csv --write-id result\example_5
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***        Example 6             ***
echo ************************************
echo ***     Configuration File       ***
echo ************************************
echo ** Converte file                  **
echo ** - parse Files                  **
echo ** - converte Files               **
echo ** - generate IDs                 **
echo ** - check double                 **
echo ** - Write IDs Header             **
echo ** - Write XML                    **
echo ** - Write CSV                    **
echo ************************************

mkdir result\example_6
copy example_6\* result\example_6

%DLT_PARSER_EXE% --no-gui --parse-cfg result\example_6\parse.cfg 
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


GOTO QUIT


:ERROR_HANDLER
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo !!!       ERROR occured          !!!
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
set /p name= Continue
exit 1


:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
set /p name= Continue
exit 0
