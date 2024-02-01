echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

REM Currently tested combinations by Qt installation
REM 5.15.2 msvc2019
REM 6.6.1 msvc2019

if "%QTVER%"=="" (
   REM set QTVER=5.15.2
   set QTVER=6.6.1
)

if "%MSVC_VER%"=="" (
    set MSVC_VER=2019
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build" (
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build"
) else (
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\BuildTools\VC\Auxiliary\Build"
)

echo Set QT directory for %QTVER% and %MSVC_VER%
if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\%QTVER%\msvc%MSVC_VER%_64
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%
