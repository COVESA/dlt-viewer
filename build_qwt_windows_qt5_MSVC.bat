@echo off
cls

REM Date     Version   Author                Changes
REM 30.1.18  1.0       Gernot Wirschal       First versioned file

echo ************************************
echo ***    Build QWT Library         ***
echo ************************************

echo ************************************
echo ***         Configuration        ***
echo ************************************


if "%MSVC_VERSION%"=="" (
   echo "Set default MSVCS ""
   set MSVC_VERSION=2015
) 

set MSVC_VER=msvc%MSVC_VERSION%

rem parameter of this batch script can be either x86 or x86_amd64

    set USE_ARCH_PARAM=false
    if "%1" NEQ "" (
        if "%1"=="x86" set USE_ARCH_PARAM=true
        if "%1"=="x86_amd64" set USE_ARCH_PARAM=true
    )
    if "!USE_ARCH_PARAM!"=="true" set ARCHITECTURE=%1
)

echo Target architecture is %ARCHITECTURE%

echo *** Setting up environment  ***

if "%MSVC_VER%"=="msvc2015" (
   set MSVC_PATH=Microsoft Visual Studio 14.0\VC
) else ( 
   set MSVC_PATH=Microsoft Visual Studio 12.0\VC
   )

echo Set QT path for %QTVER%

if "%QTVER%" EQU "5.6" (
   echo set QT_VER=Qt5.6.1\5.6
   set QT_VER=Qt5.6.1\5.6
) else ( 
   if "%QTVER%" EQU "5.5" (
   echo set QT_VER=Qt5.5.1\5.5
   set QT_VER=Qt5.5.1\5.5
   ) else ( echo set QT_VER=Qt5.8.0\5.8
            set QT_VER=Qt5.8.0\5.8
     )
 )   
   
echo Set QT diretory for %ARCHITECTURE%

  if "%ARCHITECTURE%" EQU "x86_amd64" (
    echo "Set QTDIR=C:\Qt\%QT_VER%\%MSVC_VER%_64"
    set QTDIR=C:\Qt\%QT_VER%\%MSVC_VER%_64
    )	else (
	echo "Set QTDIR=C:\Qt\%QT_VER%\%MSVC_VER%"
	set QTDIR=C:\Qt\%QT_VER%\%MSVC_VER%)
	)
  ) 


set MSVC_DIR=C:\Program Files (x86)\%MSVC_PATH%

echo Set suffix for %ARCHITECTURE%
set DIR_POSTFIX=_32bit
IF "%ARCHITECTURE%" EQU "x86_amd64" set DIR_POSTFIX=_64bit 
echo DIR_POSTFIX %DIR_POSTFIX%

set WORKINGDIR=%CD%

set QWT_DIR=C:\Qwt-%QWT%_%MSVC_VERSION%_%QTVER%%DIR_POSTFIX%


set PATH=%QTDIR%\bin;%MSVC_DIR%;%MSVC_DIR%\bin;%PATH%


IF "%WORKSPACE%" == "" (
    set SOURCE_DIR=%CD%\qwt-%QWT%
) ELSE (
    set SOURCE_DIR=%WORKSPACE%\qwt-%QWT%
)

IF '%SEVENZ_DIR%'=='' (
    set SEVENZ_DIR="C:\Program Files\7-Zip"
)

IF exist %QWT_DIR% (
echo ************************************
echo ***  Delete old qwt Directory    ***
echo ************************************

    rmdir /s /q %QWT_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * MSVC_DIR  = %MSVC_DIR%
echo * PATH      = %PATH%
echo * DLT_VIEWER_SDK_DIR = %DLT_VIEWER_SDK_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo ************************************

IF not exist "%MSVC_DIR%" (
    echo "\!"
    echo "No valid MSVC directory found in %MSVC_DIR%"
	echo "\!"
    GOTO ERROR_HANDLER
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

call vcvarsall.bat %ARCHITECTURE%
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


:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo Qwt installed in: %QWT_DIR%
cd %WORKINGDIR%
IF '%WORKSPACE%'=='' (
pause
)

