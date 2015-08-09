@echo off

echo ************************************
echo ***      DLT Viewer SDK          ***
echo ************************************

echo ************************************
echo ***         Configuration        ***
echo ************************************

echo *** Setting up environment ***

IF "%QTDIR%"=="" (
    set QTDIR=C:\Qt\Qt5.5.1\5.5\mingw492_32
)

IF "%MINGW_DIR%"=="" (
    set MINGW_DIR=C:\Qt\Qt5.5.1\Tools\mingw492_32
)

set PATH=%QTDIR%\bin;%MINGW_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

IF '%WORKSPACE%'=='' (
    IF '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=%USERPROFILE%\DltViewerSDK
    )

    set SOURCE_DIR=%CD%
    set BUILD_DIR=%CD%\build\release
) ELSE (
    IF '%DLT_VIEWER_SDK_DIR%'=='' (
        set DLT_VIEWER_SDK_DIR=%WORKSPACE%\..\GENIUS2_dlt_viewer_windows\DltViewerSDK
    )

    set SOURCE_DIR=%WORKSPACE%
    set BUILD_DIR=%WORKSPACE%\build\release
)

echo ************************************
echo * QTDIR     = %QTDIR%
echo * QTSDK     = %QTSDK%
echo * MINGW_DIR = %MINGW_DIR%
echo * PATH      = %PATH%
echo * DLT_VIEWER_SDK_DIR = %DLT_VIEWER_SDK_DIR%
echo * SOURCE_DIR         = %SOURCE_DIR%
echo * BUILD_DIR          = %BUILD_DIR%
echo ************************************

IF exist build (
echo ************************************
echo ***  Delete old build Directory  ***
echo ************************************

    rmdir /s/q build
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

    rmdir build
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

IF exist %DLT_VIEWER_SDK_DIR% (
echo ************************************
echo ***  Delete old SDK Directory    ***
echo ************************************

    rmdir /s/q %DLT_VIEWER_SDK_DIR%
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER
)

echo ************************************
echo ***        Build DLT Viewer      ***
echo ************************************

mkdir build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

cd build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

qmake ../BuildDltViewer.pro
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mingw32-make.exe -j 4 release
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo ************************************
echo ***         Create SDK           ***
echo ************************************

echo *** Create directories ***
mkdir %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\plugins\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\include
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\lib
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\filters
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\platforms
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

mkdir %DLT_VIEWER_SDK_DIR%\doc
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


mkdir %DLT_VIEWER_SDK_DIR%\cache
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

echo *** Copy files ***
copy %QTDIR%\bin\icuin54.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icuuc54.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\icudt54.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\libwinpthread-1.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\libgcc_s_dw2-1.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy "%QTDIR%\bin\libstdc++-6.dll" %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Core.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Gui.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Network.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Sql.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Svg.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Widgets.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5PrintSupport.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5Xml.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5OpenGL.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\bin\Qt5SerialPort.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %QTDIR%\plugins\platforms\qwindows.dll %DLT_VIEWER_SDK_DIR%\platforms
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


copy %BUILD_DIR%\dlt_viewer.exe %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\qdlt.dll %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltviewerplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\nonverboseplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\filetransferplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltsystemviewerplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltdbusplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\plugins\dltlogstorageplugin.dll %DLT_VIEWER_SDK_DIR%\plugins
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\doc\*.txt %DLT_VIEWER_SDK_DIR%\doc
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\README.txt %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\LICENSE.txt %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\MPL.txt %DLT_VIEWER_SDK_DIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\qdlt\*.h %DLT_VIEWER_SDK_DIR%\sdk\include
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %BUILD_DIR%\libqdlt.a %DLT_VIEWER_SDK_DIR%\sdk\lib
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummyviewerplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummydecoderplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\dummycontrolplugin %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\BuildPlugins.pro %DLT_VIEWER_SDK_DIR%\sdk\src
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummydecoderplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummydecoderplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummyviewerplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummyviewerplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\sdk\dummycontrolplugin.pro %DLT_VIEWER_SDK_DIR%\sdk\src\dummycontrolplugin
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER


copy %SOURCE_DIR%\plugin\examples\nonverboseplugin_configuration.xml %DLT_VIEWER_SDK_DIR%\plugins\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\plugin\examples\filetransferplugin_configuration.xml %DLT_VIEWER_SDK_DIR%\plugins\examples
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\filters\* %DLT_VIEWER_SDK_DIR%\filters
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

copy %SOURCE_DIR%\cache\* %DLT_VIEWER_SDK_DIR%\cache
IF %ERRORLEVEL% NEQ 0 GOTO ERROR_HANDLER

GOTO QUIT

:ERROR_HANDLER
echo ####################################
echo ###       ERROR occured          ###
echo ####################################
IF '%WORKSPACE%'=='' (
pause
)
exit 1

:QUIT
echo ************************************
echo ***       SUCCESS finish         ***
echo ************************************
echo SDK installed in: %DLT_VIEWER_SDK_DIR%
IF '%WORKSPACE%'=='' (
pause
)
exit 0
