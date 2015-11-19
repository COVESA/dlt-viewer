@echo off

echo ************************************
echo ***      DLT Viewer SDK          ***
echo ************************************

echo ************************************
echo ***         Configuration        ***
echo ************************************

setlocal EnableDelayedExpansion

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

echo Target architecture is !ARCHITECTURE!

echo *** Setting up environment ***

if "%QTDIR%"=="" (
    if "%ARCHITECTURE%"=="x86_amd64" (
        set QTDIR=C:\Qt\Qt5.5.1\5.5\msvc2013_64
    ) else (set QTDIR=C:\Qt\Qt5.5.1\5.5\msvc2013)
)

if "%MSVC_DIR%"=="" set MSVC_DIR=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC

set PATH=%QTDIR%\bin;%MSVC_DIR%;%MSVC_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

if '%WORKSPACE%'=='' (
    if '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=%USERPROFILE%\DltViewerSDK
    )

    set SOURCE_DIR=%CD%
    set BUILD_DIR=%CD%\build\release
) else (
    if '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=%WORKSPACE%\..\GENIUS2_dlt_viewer_windows\DltViewerSDK
    )

    set SOURCE_DIR=%WORKSPACE%
    set BUILD_DIR=%WORKSPACE%\build\release
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * QTSDK     = %QTSDK%
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

    rmdir /s/q build
    if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

    rmdir build
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

    rmdir /s/q %DLT_VIEWER_SDK_DIR%
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

echo *** Create directories ***
mkdir %DLT_VIEWER_SDK_DIR%
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
copy %QTDIR%\bin\icuin54.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icuuc54.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icudt54.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

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

copy %QTDIR%\plugins\platforms\qwindows.dll %DLT_VIEWER_SDK_DIR%\platforms
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\dlt_viewer.exe %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qextserialport.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qdlt.dll %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

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
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %DLT_VIEWER_SDK_DIR%
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\README.txt %DLT_VIEWER_SDK_DIR%
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

copy %BUILD_DIR%\qextserialport.dll %DLT_VIEWER_SDK_DIR%\sdk\lib
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qextserialport.lib %DLT_VIEWER_SDK_DIR%\sdk\lib
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummyviewerplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummydecoderplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummycontrolplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\BuildPlugins.pro %DLT_VIEWER_SDK_DIR%\sdk\src
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummydecoderplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

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

copy %SOURCE_DIR%\cache\* %DLT_VIEWER_SDK_DIR%\cache
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

GOTO QUIT

:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
if '%WORKSPACE%'=='' (
pause
)
exit 1

:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo SDK installed in: %DLT_VIEWER_SDK_DIR%
if '%WORKSPACE%'=='' (
pause
)
exit 0
