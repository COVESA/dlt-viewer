echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

REM Currently supported combinations by Qt installation
REM 5.12.12 msvc2015,msvc2017
REM 5.15.2 msvc2015,msvc2019
REM 6.2.2 msvc2019

IF "%QTVER%"=="" (
REM    set QTVER=5.12.12
    set QTVER=5.15.2
REM    set QTVER=6.2.2
)

IF "%MSVC_VER%"=="" (
REM	   set MSVC_VER=2015
REM    set MSVC_VER=2017
	set MSVC_VER=2019
)

echo Set QT directory for %QTVER% and %MSVC_VER%
if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\%QTVER%\msvc%MSVC_VER%_64
)

if "%MSVC_VER%"=="2015" (
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
) else (
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\BuildTools\VC\Auxiliary\Build"
)

IF "%QWT%"=="" (
REM    set QWT=6.1.4
    set QWT=6.1.6
REM    set QWT=6.2.0
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%
