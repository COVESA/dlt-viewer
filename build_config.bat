echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

REM Currently tested combinations by Qt installation
REM 5.15.2 msvc2019
REM 6.7.3 msvc2019

if "%QTNO%"=="" (
   set QTNO=6
)

if "%QTVER%"=="" (
   if "%QTNO%"=="5" (
		set QTVER=5.15.2 
   )  else (
		set QTVER=6.7.3
   )
)

if "%MSVC_VER%"=="" (
    set MSVC_VER=2019
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build" (
    REM Visual Studio Community Edition 2019
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\%MSVC_VER%\Enterprise\VC\Auxiliary\Build"
) else (
    REM Vidual Studio Professional 2019
	if "%MSVC_DIR%"=="" set "MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build"
)

echo Set QT directory for %QTVER% and %MSVC_VER%
if "%QTDIR%"=="" (
    set QTDIR=C:\Qt\%QTVER%\msvc%MSVC_VER%_64
)

set PATH=%QTDIR%\bin;%MSVC_DIR%;%PATH%
