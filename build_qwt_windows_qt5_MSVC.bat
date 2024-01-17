@echo off
cls

REM Date     Version   Author                Changes
REM 30.1.18  1.0       Gernot Wirschal       First versioned file
REM 4.7.19   1.1       Alexander Wenzel      Update to Qt 5.12.4, Qwt 6.1.4 and Visual Studio 2015
REM 11.1.21  1.2       Alexander Wenzel      Update to Qt 5.12.12, Visual Studio 2017 Build Tools and simplify

echo ************************************
echo ***    Build QWT Library         ***
echo ************************************

call build_config.bat

set WORKINGDIR=%CD%

if '%WORKSPACE%'=='' (
    IF "%QWT_DIR%"=="" (
        set QWT_DIR=C:\Qwt-%QWT%_%QTVER%
    )

    set SOURCE_DIR=%CD%\qwt-%QWT%
) else (
    if '%QWT_DIR%'=='' (
        set QWT_DIR=%WORKSPACE%\Qwt-%QWT%_%QTVER%
    )

    set SOURCE_DIR=%WORKSPACE%\qwt-%QWT%
)

IF '%SEVENZ_DIR%'=='' (
    set SEVENZ_DIR="C:\Program Files\7-Zip"
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * MSVC_DIR  = %MSVC_DIR%
echo * PATH      = %PATH%
echo * QWT_DIR   = %QWT_DIR%
echo * SOURCE_DIR = %SOURCE_DIR%
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

call vcvarsall.bat x86_amd64
if %ERRORLEVEL% NEQ 0 goto error
echo configuring was successful

echo ************************************
echo ***          Build QWT           ***
echo ************************************

set QMAKEFEATURES=%QWT_DIR%\features

qmake qwt.pro
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake clean

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

