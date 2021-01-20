@echo off

REM Date     Version   Author                Changes
REM 4.7.19   1.0       Alexander Wenzel      Update to Qt 5.12.4 and Visual Studio 2015
REM 25.11.20 1.2       Alexander Wenzel      Update to Qt 5.12.10

echo ************************************
echo ***      DLT Parser              ***
echo ************************************

echo .
echo ************************************
echo ***         Configuration        ***
echo ************************************

rem setlocal enabledelayedexpansion

rem parameter of this batch script can be either x86 or x86_amd64
if "%ARCHITECTURE%"=="" (
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set ARCHITECTURE=x86_amd64
    ) else (
        set ARCHITECTURE=x86
    )

    set USE_ARCH_PARAM=false
    if "%1" NEQ "" (
        if "%1"=="x86" set USE_ARCH_PARAM=true
        if "%1"=="x86_amd64" set USE_ARCH_PARAM=true
    )
    if "!USE_ARCH_PARAM!"=="true" set ARCHITECTURE=%1
)

echo Target architecture is !ARCHITECTURE!

echo *** Setting up environment ***

if "%QTDIR%"=="" (
    if "%ARCHITECTURE%"=="x86_amd64" (
        set QTDIR=C:\Qt\Qt5.12.10\5.12.10\msvc2015_64
    ) else (set QTDIR=C:\Qt\Qt5.12.10\5.12.10\msvc2015)
)

if "%MSVC_DIR%"=="" set MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC

set PATH=%QTDIR%\bin;%MSVC_DIR%;%MSVC_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

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
echo * QTSDK     = %QTSDK%
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

call vcvarsall.bat %ARCHITECTURE%
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
rem mkdir %DLT_PARSER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

rem mkdir %DLT_PARSER_DIR%\examples
if not exist %DLT_PARSER_DIR%\examples mkdir %DLT_PARSER_DIR%\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Copy files ***
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
 
copy %QTDIR%\plugins\platforms\qwindows.dll %DLT_PARSER_DIR%\platforms
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
