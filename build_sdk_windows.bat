echo off
echo *************************
echo * Configuration         *
echo *************************

echo Setting up environment for Qt usage...
set QTSDK=C:\Qt\4.8.2
set QTDIR=C:\Qt\4.8.2
set PATH=%QTDIR%\bin;%PATH%
set PATH=C:\MinGW\bin;%PATH%

echo Setting up environment for DLT Viewer SDK...
set SDK_DIR=c:\DltViewerSDK
set PWD=%~dp0
set MINGW_DIR=C:\MinGW
set SOURCE_DIR=%PWD%
set BUILD_DIR=%PWD%build\release

echo *************************
echo * Build DLT Viewer      *
echo *************************

mkdir build
cd build
qmake ../BuildDltViewer.pro
mingw32-make.exe release

echo *************************
echo * Create SDK            *
echo *************************

ECHO Create directories
mkdir %SDK_DIR%
mkdir %SDK_DIR%\plugins
mkdir %SDK_DIR%\plugins\examples
mkdir %SDK_DIR%\sdk
mkdir %SDK_DIR%\sdk\include
mkdir %SDK_DIR%\sdk\lib
mkdir %SDK_DIR%\sdk\src
mkdir %SDK_DIR%\sdk\src\dummydecoderplugin
mkdir %SDK_DIR%\sdk\src\dummyviewerplugin
mkdir %SDK_DIR%\sdk\src\dummycontrolplugin

ECHO Copy files
copy %MINGW_DIR%\bin\mingwm10.dll %SDK_DIR%
copy %MINGW_DIR%\bin\libgcc_s_dw2-1.dll %SDK_DIR%
copy "%MINGW_DIR%\bin\libstdc++-6.dll" %SDK_DIR%
copy %QTDIR%\bin\QtCore4.dll %SDK_DIR%
copy %QTDIR%\bin\QtGui4.dll %SDK_DIR%
copy %QTDIR%\bin\QtNetwork4.dll %SDK_DIR%
copy %QTDIR%\bin\QtSql4.dll %SDK_DIR%
copy %QTDIR%\bin\QtSvg4.dll %SDK_DIR%
copy %QTDIR%\bin\QtXml4.dll %SDK_DIR%

copy %BUILD_DIR%\dlt_viewer.exe %SDK_DIR%
copy %BUILD_DIR%\qextserialport.dll %SDK_DIR%
copy %BUILD_DIR%\qdlt.dll %SDK_DIR%

copy %BUILD_DIR%\plugins\dltviewerplugin.dll %SDK_DIR%\plugins
copy %BUILD_DIR%\plugins\nonverboseplugin.dll %SDK_DIR%\plugins
copy %BUILD_DIR%\plugins\filetransferplugin.dll %SDK_DIR%\plugins


copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %SDK_DIR%
copy %SOURCE_DIR%\README.txt %SDK_DIR%
copy %SOURCE_DIR%\LICENSE.txt %SDK_DIR%
copy %SOURCE_DIR%\MPL.txt %SDK_DIR%

copy %SOURCE_DIR%\qdlt\dlt.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\dlt_common.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\dlt_protocol.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\dlt_types.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\dlt_user.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\dlt_user_macros.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\qdlt\qdlt.h %SDK_DIR%\sdk\include
copy %SOURCE_DIR%\src\plugininterface.h %SDK_DIR%\sdk\include

copy %BUILD_DIR%\libqdlt.a %SDK_DIR%\sdk\lib
copy %BUILD_DIR%\libqextserialport.a %SDK_DIR%\sdk\lib

copy %SOURCE_DIR%\plugin\dummyviewerplugin %SDK_DIR%\sdk\src\dummyviewerplugin
copy %SOURCE_DIR%\plugin\dummydecoderplugin %SDK_DIR%\sdk\src\dummydecoderplugin
copy %SOURCE_DIR%\plugin\dummycontrolplugin %SDK_DIR%\sdk\src\dummycontrolplugin

copy %SOURCE_DIR%\sdk\BuildPlugins.pro %SDK_DIR%\sdk\src
copy %SOURCE_DIR%\sdk\dummydecoderplugin.pro %SDK_DIR%\sdk\src\dummydecoderplugin
copy %SOURCE_DIR%\sdk\dummyviewerplugin.pro %SDK_DIR%\sdk\src\dummyviewerplugin
copy %SOURCE_DIR%\sdk\dummycontrolplugin.pro %SDK_DIR%\sdk\src\dummycontrolplugin

copy %SOURCE_DIR%\plugin\examples\nonverboseplugin_configuration.xml %SDK_DIR%\plugins\examples
copy %SOURCE_DIR%\plugin\examples\filetransferplugin_configuration.xml %SDK_DIR%\plugins\examples

echo *************************
echo * Finish                *
echo *************************
echo SDK installed in:  %SDK_DIR%
set /p name= Continue