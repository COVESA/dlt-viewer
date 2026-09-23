echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

REM Currently tested combinations by Qt installation
REM 6.8.3 msvc2022

if "%QTNO%"=="" (
   set QTNO=6
)

if "%QTVER%"=="" (
    set QTVER=6.8.3
)

if "%MSVC_VER%"=="" (
    set MSVC_VER=2022
)

REM vswhere.exe (ships on GitHub-hosted runners) reliably finds VS regardless of edition/path.
REM Kept as its own statements (not nested in one if-block) so %VSWHERE%/%VS_FOUND_DIR% expand correctly.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

REM Use a non-reserved variable name: VSINSTALLDIR is set/read internally by vcvarsall.bat/VsDevCmd.bat,
REM and pre-setting it here confuses that script into thinking the environment is already initialized.
set "VS_FOUND_DIR="
if "%MSVC_DIR%"=="" if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_FOUND_DIR=%%i"
)

if "%MSVC_DIR%"=="" if defined VS_FOUND_DIR set "MSVC_DIR=%VS_FOUND_DIR%\VC\Auxiliary\Build"
set "VS_FOUND_DIR="

if exist "C:\Program Files\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build" (
    REM Visual Studio Community Edition
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Professional\VC\Auxiliary\Build" (
    REM Visual Studio Professional
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Professional\VC\Auxiliary\Build"
) else (
    REM Visual Studio Professional
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files\Microsoft Visual Studio\%MSVC_VER%\Professional\VC\Auxiliary\Build"
)

echo Set QT directory for %QTVER% and %MSVC_VER%
if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\%QTVER%\msvc%MSVC_VER%_64
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%
