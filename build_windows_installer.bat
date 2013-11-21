echo off

echo ************************************
echo ***     DLT Viewer Installer     ***
echo ************************************

echo .
echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***
 
IF "%DLT_VIEWER_SDK_DIR%"=="" (
        set DLT_VIEWER_SDK_DIR=C:\DltViewerSDK
)

set SOURCE_DIR=%CD%
set MAKENSIS_PATH="C:\Program Files\NSIS\makensis.exe"
set NSISCONFDIR=%SOURCE_DIR%\installer
set INSTALLER_DIR=%DLT_VIEWER_SDK_DIR%\installer

echo ************************************
echo * DLT_VIEWER_SDK_DIR = %DLT_VIEWER_SDK_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo * MAKENSIS_PATH      = %MAKENSIS_PATH%
echo * NSISCONFDIR        = %NSISCONFDIR%
echo * INSTALLER_DIR      = %INSTALLER_DIR%
echo ************************************

IF exist %INSTALLER_DIR% (
echo ************************************
echo ***  Delete old installer        ***
echo ************************************

    rmdir /s/q %INSTALLER_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

)

echo ***********************************
echo ***    Creating Installer       ***
echo ***********************************

mkdir %INSTALLER_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

%MAKENSIS_PATH% %NSISCONFDIR%\dlt_installer.nsi
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
echo Installer created in: %INSTALLER_DIR%
set /p name= Continue
exit 0
