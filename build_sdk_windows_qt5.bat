echo off
echo *************************
echo * Configuration         *
echo *************************

echo Setting up environment for Qt usage...

set QTDIR=C:\Qt\Qt5.1.1\5.1.1\mingw48_32
set MINGW_DIR=C:\Qt\Qt5.1.1\Tools\mingw48_32

set PATH=%QTDIR%\bin;%MINGW_DIR%\bin;%PATH%
set QTSDK=%QTDIR%

echo *************************
echo * QTDIR = %QTDIR%
echo * MINGW_DIR = %MINGW_DIR%
echo *************************

echo Setting up environment for DLT Viewer SDK...

set SDK_DIR=c:\DltViewerSDK
set PWD=%~dp0
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

echo Create directories
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
mkdir %SDK_DIR%\filters
mkdir %SDK_DIR%\platforms
mkdir %SDK_DIR%\cache

echo Copy files
copy %QTDIR%\bin\icuin51.dll %SDK_DIR%
copy %QTDIR%\bin\icuuc51.dll %SDK_DIR%
copy %QTDIR%\bin\icudt51.dll %SDK_DIR%
copy %QTDIR%\bin\libwinpthread-1.dll %SDK_DIR%
copy %QTDIR%\bin\libgcc_s_dw2-1.dll %SDK_DIR%
copy "%QTDIR%\bin\libstdc++-6.dll" %SDK_DIR%
copy %QTDIR%\bin\Qt5Core.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Gui.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Network.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Sql.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Svg.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Widgets.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5PrintSupport.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5Xml.dll %SDK_DIR%
copy %QTDIR%\bin\Qt5OpenGL.dll %SDK_DIR%
copy %QTDIR%\plugins\platforms\qwindows.dll %SDK_DIR%\platforms

copy %BUILD_DIR%\dlt_viewer.exe %SDK_DIR%
copy %BUILD_DIR%\qextserialport.dll %SDK_DIR%
copy %BUILD_DIR%\qdlt.dll %SDK_DIR%

copy %BUILD_DIR%\plugins\dltviewerplugin.dll %SDK_DIR%\plugins
copy %BUILD_DIR%\plugins\nonverboseplugin.dll %SDK_DIR%\plugins
copy %BUILD_DIR%\plugins\filetransferplugin.dll %SDK_DIR%\plugins
copy %BUILD_DIR%\plugins\dltsystemviewerplugin.dll %SDK_DIR%\plugins

copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %SDK_DIR%
copy %SOURCE_DIR%\README.txt %SDK_DIR%
copy %SOURCE_DIR%\LICENSE.txt %SDK_DIR%
copy %SOURCE_DIR%\MPL.txt %SDK_DIR%

copy %SOURCE_DIR%\qdlt\*.h %SDK_DIR%\sdk\include

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

copy %SOURCE_DIR%\filters\* %SDK_DIR%\filters
copy %SOURCE_DIR%\cache\* %SDK_DIR%\cache

echo *************************
echo * Finish                *
echo *************************
echo SDK installed in:  %SDK_DIR%
set /p name= Continue
