echo off
echo *************************
echo * Configuration         *
echo *************************

echo Setting up environment for DLT Viewer Installer...
echo Instructions are described in the "Create Installer" section of the INSTALL.txt
 
set SDK_DIR=c:\DltViewerSDK
set PWD=%~dp0
set SOURCE_DIR=%PWD%
set MAKENSIS_PATH="C:\Program Files (x86)\NSIS\makensis.exe"
set NSISCONFDIR=%SOURCE_DIR%\installer\

echo *************************
echo * Creating Installer    *
echo *************************

mkdir %SDK_DIR%\installer\

%MAKENSIS_PATH% %NSISCONFDIR%\dlt_installer.nsi


echo *************************
echo * Finish                *
echo *************************
echo Installer created in: %SDK_DIR%\installer
set /p name= Continue