echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

if "%QTVER%"=="" (
REM set QTVER=6.4.3
REM set QTVER=6.5.3
set QTVER=6.6.1
)

if "%MSVC_VER%"=="" (
    set MSVC_VER=2019
)

if "%MSVC_DIR%"=="" (
	set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\BuildTools\VC\Auxiliary\Build"
)

echo Set QT directory for %QTVER% and %MSVC_VER%
if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\%QTVER%\msvc%MSVC_VER%_64
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%
