@echo off

REM Date     Version   Author                Changes
REM 4.7.19   1.0       Alexander Wenzel      Update to Qt 5.12.4 and Visual Studio 2015
REM 25.11.20 1.1       Alexander Wenzel      Update to Qt 5.12.10

echo ************************************
echo ***      DLT Viewer SDK          ***
echo ************************************
echo ************************************
echo ***         Configuration        ***
echo ************************************

rem parameter of this batch script can be either x86 or x86_amd64
if "%ARCHITECTURE%"=="" (
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set ARCHITECTURE=x86_amd64
    ) else (
        set ARCHITECTURE=x86
    )

    set USE_ARCH_PARAM=false
    if "%1" NEQ "" (
        if "%1"=="x86" set USE_ARCH_PARAM=true
        if "%1"=="x86_amd64" set USE_ARCH_PARAM=true
    )
    if "!USE_ARCH_PARAM!"=="true" set ARCHITECTURE=%1
)

echo Target architecture is %ARCHITECTURE%

echo *** Setting up environment ***

if "%QTDIR%"=="" (
    if "%ARCHITECTURE%"=="x86_amd64" (
        set QTDIR=C:\Qt\Qt5.12.10\5.12.10\msvc2015_64
    ) else (set QTDIR=C:\Qt\Qt5.12.10\5.12.10\msvc2015)
)

if "%MSVC_DIR%"=="" set MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC


set PATH=%QTDIR%\bin;%MSVC_DIR%;%MSVC_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

if '%WORKSPACE%'=='' (
    if '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=c:\DltViewerSDK
    )

    set SOURCE_DIR=%CD%
    set BUILD_DIR=%CD%\build\release
) else (
    if '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=%WORKSPACE%\build\dist\DltViewerSDK
    )

    set SOURCE_DIR=%WORKSPACE%
    set BUILD_DIR=%WORKSPACE%\build\release
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * QTSDK     = %QTSDK%
echo * QWT_DIR   = %QWT_DIR%
echo * MSVC_DIR  = %MSVC_DIR%
echo * PATH      = %PATH%
echo * DLT_VIEWER_SDK_DIR = %DLT_VIEWER_SDK_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo * BUILD_DIR          = %BUILD_DIR%
echo ************************************

if exist build (
echo ************************************
echo ***  Delete old build Directory  ***
echo ************************************

    rmdir /s /q build
    if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

)

echo ************************************
echo ***  Configure MSVC environment  ***
echo ************************************

call vcvarsall.bat %ARCHITECTURE%
if %ERRORLEVEL% NEQ 0 goto error
echo configuring was successful

if exist %DLT_VIEWER_SDK_DIR% (
echo ************************************
echo ***   Delete old SDK Directory   ***
echo ************************************

    rmdir /s /q %DLT_VIEWER_SDK_DIR%
    if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***       Build DLT Viewer       ***
echo ************************************

mkdir build
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd build
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

qmake ../BuildDltViewer.pro
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

nmake
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***         Create SDK           ***
echo ************************************

if not exist %DLT_VIEWER_SDK_DIR% mkdir %DLT_VIEWER_SDK_DIR%
echo *** Create directories %DLT_VIEWER_SDK_DIR% ***
rem mkdir %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\plugins\examples
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\include
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\lib
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\filters
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\platforms
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\doc
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\cache
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Copy files ***
copy %QTDIR%\bin\Qt5Core.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Gui.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Network.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Sql.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Svg.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Widgets.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5PrintSupport.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Xml.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5OpenGL.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5SerialPort.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\plugins\platforms\qwindows.dll %DLT_VIEWER_SDK_DIR%\platforms
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\dlt-viewer.exe %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qdlt.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

rem echo "copy %QWT_DIR%\lib\qwt.dll %DLT_VIEWER_SDK_DIR%"
rem copy %QWT_DIR%\lib\qwt.dll %DLT_VIEWER_SDK_DIR%
rem IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

rem copy %BUILD_DIR%\plugins\speedplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltviewerplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\nonverboseplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\filetransferplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltsystemviewerplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltdbusplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltlogstorageplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\doc\*.txt %DLT_VIEWER_SDK_DIR%\doc
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %DLT_VIEWER_SDK_DIR%
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\README.md %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\LICENSE.txt %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\MPL.txt %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\qdlt\*.h %DLT_VIEWER_SDK_DIR%\sdk\include
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qdlt.dll %DLT_VIEWER_SDK_DIR%\sdk\lib
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qdlt.lib %DLT_VIEWER_SDK_DIR%\sdk\lib
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummyviewerplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummydecoderplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummycontrolplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\BuildPlugins.pro %DLT_VIEWER_SDK_DIR%\sdk\src
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummydecoderplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
rem if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummyviewerplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummycontrolplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\examples\nonverboseplugin_configuration.xml %DLT_VIEWER_SDK_DIR%\plugins\examples
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\examples\filetransferplugin_configuration.xml %DLT_VIEWER_SDK_DIR%\plugins\examples
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\filters\* %DLT_VIEWER_SDK_DIR%\filters
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


GOTO QUIT

:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
cd ..
exit 1

:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
cd ..
echo SDK installed in: %DLT_VIEWER_SDK_DIR%

exit 0
