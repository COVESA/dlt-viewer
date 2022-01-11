@echo off
cls

REM Date     Version   Author                Changes
REM 30.1.18  1.0       Gernot Wirschal       First versioned file
REM 4.7.19   1.1       Alexander Wenzel      Update to Qt 5.12.4, Qwt 6.1.4 and Visual Studio 2015
REM 25.11.20 1.2       Alexander Wenzel      Update to Qt 5.12.10
REM 11.1.21  1.3       Alexander Wenzel      Update to Qt 5.12.12, Visual Studio 2017 Build Tools, Qwt 6.2.0 and simplify

echo ************************************
echo ***    Build QWT Library         ***
echo ************************************

echo ************************************
echo ***         Configuration        ***
echo ************************************

rem parameter of this batch script can be either x86 or x86_amd64

echo *** Setting up environment  ***

IF "%QTVER%"=="" (
    set QTVER=5.12.12
)

echo Set QT path for %QTVER%

if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\Qt%QTVER%\%QTVER%\msvc2017_64
)

if "%MSVC_DIR%"=="" set MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build

set WORKINGDIR=%CD%

IF "%QWT%"=="" (
    set QWT=6.2.0
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%

IF "%QWT_DIR%"=="" (
	set QWT_DIR=C:\Qwt-%QWT%_%QTVER%
)

set SOURCE_DIR=%CD%\qwt-%QWT%

IF '%SEVENZ_DIR%'=='' (
    set SEVENZ_DIR="C:\Program Files\7-Zip"
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * MSVC_DIR  = %MSVC_DIR%
echo * PATH      = %PATH%
echo * QWT_DIR = %QWT_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo ************************************

IF not exist "%MSVC_DIR%" (
    echo "\!"
    echo "No valid MSVC directory found in %MSVC_DIR%"
	echo "\!"
    GOTO ERROR_HANDLER
)

IF exist %QWT_DIR% (
echo ************************************
echo ***  Delete old qwt Directory    ***
echo ************************************

    rmdir /s /q %QWT_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***        Unzip Archives        ***
echo ************************************

IF exist qwt-%QWT% (
     
    rmdir /s /q qwt-%QWT%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

%SEVENZ_DIR%\7z.exe x qwt-%QWT%.zip -y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd %SOURCE_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***  Configure MSVC environment  ***
echo ************************************

call vcvars64.bat
if %ERRORLEVEL% NEQ 0 goto error
echo configuring was successful

echo ************************************
echo ***          Build QWT           ***
echo ************************************

set QMAKEFEATURES=%QWT_DIR%\features

qmake qwt.pro
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake clean
rem IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake install
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Move QWT folder ***
move "C:\Qwt-%QWT%" %QWT_DIR%

GOTO QUIT

:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
cd %WORKINGDIR%
IF '%WORKSPACE%'=='' (
    pause
)
exit 1


:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo Qwt installed in: %QWT_DIR%
cd %WORKINGDIR%
IF '%WORKSPACE%'=='' (
    pause
)

