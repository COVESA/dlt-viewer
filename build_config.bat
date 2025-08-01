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
