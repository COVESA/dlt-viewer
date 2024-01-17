@echo off

REM Date     Version   Author                Changes
REM 4.7.19   1.0       Alexander Wenzel      Update to Qt 5.12.4 and Visual Studio 2015
REM 25.11.20 1.1       Alexander Wenzel      Update to Qt 5.12.10
REM 11.1.21  1.2       Alexander Wenzel      Update to Qt 5.12.12, Visual Studio 2017 Build Tools and simplify

echo ************************************
echo ***      DLT Parser              ***
echo ************************************

call build_config.bat

if '%WORKSPACE%'=='' (
    if '%DLT_PARSER_DIR%'=='' (
        set DLT_PARSER_DIR=c:\DltParser
    )

    set SOURCE_DIR=%CD%
    set BUILD_DIR=%CD%\build\release
) else (
    if '%DLT_PARSER_DIR%'=='' (
        set DLT_PARSER_DIR=%WORKSPACE%\build\dist\DltParser
    )

    set SOURCE_DIR=%WORKSPACE%
    set BUILD_DIR=%WORKSPACE%\build\release
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * MSVC_DIR  = %MSVC_DIR%
echo * PATH      = %PATH%
echo * DLT_PARSER_DIR = %DLT_PARSER_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo * BUILD_DIR          = %BUILD_DIR%
echo ************************************

IF exist build (
echo ************************************
echo ***  Delete old build Directory  ***
echo ************************************
    rmdir /S /Q build
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

IF exist %DLT_PARSER_DIR% (
echo ************************************
echo ***  Delete old SDK Directory    ***
echo ************************************

    rmdir /s /q %DLT_PARSER_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***  Configure MSVC environment  ***
echo ************************************

call vcvarsall.bat x86_amd64
if %ERRORLEVEL% NEQ 0 goto error
echo configuring was successful

echo ************************************
echo ***        Build DLT Parser      ***
echo ************************************

if not exist build mkdir build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

qmake ../BuildDltParser.pro
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake release
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***         Install Parser       ***
echo ************************************

echo *** Create directories %DLT_PARSER_DIR% ***
if not exist %DLT_PARSER_DIR% mkdir %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

if not exist %DLT_PARSER_DIR%\examples mkdir %DLT_PARSER_DIR%\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Copy files ***
copy %QTDIR%\bin\Qt?Core.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Gui.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Network.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Sql.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Svg.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Widgets.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?PrintSupport.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?Xml.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt?OpenGL.dll %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\dlt-parser.exe %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\ReleaseNotes_Parser.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\README_Parser.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\LICENSE.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\MPL.txt %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_PARSER_DIR%\platforms 
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
 
mkdir %DLT_PARSER_DIR%\styles 
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
 
copy %QTDIR%\plugins\platforms\qwindows.dll %DLT_PARSER_DIR%\platforms
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\plugins\styles\qwindowsvistastyle.dll %DLT_PARSER_DIR%\styles
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


xcopy %SOURCE_DIR%\parser\examples %DLT_PARSER_DIR%\examples /E
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

GOTO QUIT


:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
set /p name= Continue
exit 1


:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo SDK installed in: %DLT_PARSER_DIR%
set /p name= Continue
exit 0
