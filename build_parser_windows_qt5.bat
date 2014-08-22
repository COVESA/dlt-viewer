echo off

echo ************************************
echo ***      DLT Parser              ***
echo ************************************

echo .
echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

IF "%QTDIR%"=="" (
    set QTDIR=C:\Qt\Qt5.3.1\5.3\mingw482_32
)

IF "%MINGW_DIR%"=="" (
    set MINGW_DIR=C:\Qt\Qt5.3.1\Tools\mingw482_32
)

set PATH=%QTDIR%\bin;%MINGW_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

IF "%DLT_PARSER_DIR%"=="" (
        set DLT_PARSER_DIR=c:\DltParser
)

set SOURCE_DIR=%CD%
set BUILD_DIR=%CD%\build\release

echo ************************************
echo * QTDIR     = %QTDIR%
echo * QTSDK     = %QTSDK%
echo * MINGW_DIR = %MINGW_DIR%
echo * PATH      = %PATH%
echo * DLT_PARSER_DIR = %DLT_PARSER_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo * BUILD_DIR          = %BUILD_DIR%
echo ************************************

IF exist build (
echo ************************************
echo ***  Delete old build Directory  ***
echo ************************************

    rmdir /s/q build
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

    rmdir build
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

IF exist %DLT_PARSER_DIR% (
echo ************************************
echo ***  Delete old SDK Directory    ***
echo ************************************

    rmdir /s/q %DLT_PARSER_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***        Build DLT Parser      ***
echo ************************************

mkdir build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

qmake ../BuildDltParser.pro
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mingw32-make.exe release
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***         Install Parser       ***
echo ************************************

echo *** Create directories ***
mkdir %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_PARSER_DIR%\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Copy files ***
copy %QTDIR%\bin\icuin52.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icuuc52.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icudt52.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\libwinpthread-1.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\libgcc_s_dw2-1.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy "%QTDIR%\bin\libstdc++-6.dll" %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Core.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Gui.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Network.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Sql.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Svg.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Widgets.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5PrintSupport.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Xml.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5OpenGL.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\dlt_parser.exe %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


copy %SOURCE_DIR%\ReleaseNotes_Parser.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\README_Parser.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\LICENSE.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\MPL.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

xcopy %SOURCE_DIR%\parser\examples %DLT_PARSER_DIR%\examples /E
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
echo SDK installed in: %DLT_PARSER_DIR%
set /p name= Continue
exit 0
