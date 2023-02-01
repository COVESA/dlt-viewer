set MINGW_DIR=C:\MinGW
set QT_DIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw\bin
set SOURCE_DIR=C:\data\git\DLT-viewer
set BUILD_DIR=C:\data\git\viewer-release\release
set TARGET_DIR=c:\DltViewerSDK

cd %BUILD_DIR%

ECHO Create directories
mkdir %TARGET_DIR%
mkdir %TARGET_DIR%\plugins
mkdir %TARGET_DIR%\sdk
mkdir %TARGET_DIR%\sdk\include
mkdir %TARGET_DIR%\sdk\include\qdlt
mkdir %TARGET_DIR%\sdk\lib
mkdir %TARGET_DIR%\sdk\src
mkdir %TARGET_DIR%\sdk\src\dummydecoderplugin
mkdir %TARGET_DIR%\sdk\src\dummyviewerplugin
mkdir %TARGET_DIR%\sdk\src\dummycontrolplugin

ECHO Copy files
copy %MINGW_DIR%\bin\mingwm10.dll %TARGET_DIR%
copy %MINGW_DIR%\bin\libgcc_s_dw2-1.dll %TARGET_DIR%
copy "%MINGW_DIR%\bin\libstdc++-6.dll" %TARGET_DIR%
copy %QT_DIR%\QtCore4.dll %TARGET_DIR%
copy %QT_DIR%\QtGui4.dll %TARGET_DIR%
copy %QT_DIR%\QtNetwork4.dll %TARGET_DIR%

copy %BUILD_DIR%\dlt-viewer.exe %TARGET_DIR%
copy %BUILD_DIR%\qdlt.dll %TARGET_DIR%

copy %BUILD_DIR%\plugins\dltviewerplugin.dll %TARGET_DIR%\plugins
copy %BUILD_DIR%\plugins\nonverboseplugin.dll %TARGET_DIR%\plugins
copy %BUILD_DIR%\plugins\filetransferplugin.dll %TARGET_DIR%\plugins
copy %BUILD_DIR%\plugins\dltsystemviewerplugin.dll %TARGET_DIR%\plugins

copy %SOURCE_DIR%\ReleaseNotes_Viewer.txt %TARGET_DIR%

copy %SOURCE_DIR%\qdlt\dlt.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\dlt_common.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\dlt_protocol.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\dlt_types.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\dlt_user.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\dlt_user_macros.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\qdlt\qdlt.h %TARGET_DIR%\sdk\include\qdlt
copy %SOURCE_DIR%\src\plugininterface.h %TARGET_DIR%\sdk\include\qdlt

copy %BUILD_DIR%\libqdlt.a %TARGET_DIR%\sdk\lib

copy %SOURCE_DIR%\plugin\dummyviewerplugin %TARGET_DIR%\sdk\src\dummyviewerplugin
copy %SOURCE_DIR%\plugin\dummydecoderplugin %TARGET_DIR%\sdk\src\dummydecoderplugin
copy %SOURCE_DIR%\plugin\dummycontrolplugin %TARGET_DIR%\sdk\src\dummycontrolplugin

copy %SOURCE_DIR%\sdk\BuildPlugins.pro %TARGET_DIR%\sdk\src
copy %SOURCE_DIR%\sdk\dummydecoderplugin.pro %TARGET_DIR%\sdk\src\dummydecoderplugin
copy %SOURCE_DIR%\sdk\dummyviewerplugin.pro %TARGET_DIR%\sdk\src\dummyviewerplugin
copy %SOURCE_DIR%\sdk\dummycontrolplugin.pro %TARGET_DIR%\sdk\src\dummycontrolplugin
