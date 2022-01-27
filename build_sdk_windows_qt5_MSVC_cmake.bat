@echo off

REM Date     Version   Author                Changes
REM 4.7.19   1.0       Alexander Wenzel      Update to Qt 5.12.4 and Visual Studio 2015
REM 25.11.20 1.1       Alexander Wenzel      Update to Qt 5.12.10
REM 11.1.21  1.2       Alexander Wenzel      Update to Qt 5.12.12, Visual Studio 2017 Build Tools, simplify and cmake
REM 27.1.21  1.2       Alexander Wenzel      Update using cmake install

echo ************************************
echo ***      DLT Viewer SDK (cmake)  ***
echo ************************************

call build_config.bat

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

    rmdir /s /q build || rem
    if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

)

echo ************************************
echo ***  Configure MSVC environment  ***
echo ************************************

call vcvarsall.bat x86_amd64
if %ERRORLEVEL% NEQ 0 goto ERROR_HANDLER
echo configuring was successful

if exist %DLT_VIEWER_SDK_DIR% (
echo ************************************
echo ***   Delete old SDK Directory   ***
echo ************************************

    rmdir /s /q %DLT_VIEWER_SDK_DIR% || rem
    if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***       Build DLT Viewer       ***
echo ************************************

mkdir build
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd build
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir Release
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd Release
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%DLT_VIEWER_SDK_DIR% ..\..
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cmake --build .
if %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***         Create SDK           ***
echo ************************************

cmake --install .

GOTO QUIT

:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
exit /b 1


:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo SDK installed in: %DLT_VIEWER_SDK_DIR%
exit /b 0
