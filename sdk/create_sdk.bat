set TARGET_DIR=dlt_viewer_new_release

cd ..\..\BuildDltViewer-build-desktop\release

ECHO Create directories
mkdir %TARGET_DIR%
mkdir %TARGET_DIR%\plugins
mkdir %TARGET_DIR%\sdk
mkdir %TARGET_DIR%\sdk\include
mkdir %TARGET_DIR%\sdk\lib
mkdir %TARGET_DIR%\sdk\src
mkdir %TARGET_DIR%\sdk\src\dummydecoderplugin
mkdir %TARGET_DIR%\sdk\src\dummyviewerplugin
mkdir %TARGET_DIR%\sdk\src\dummycontrolplugin

ECHO Copy files
copy mingwm10.dll %TARGET_DIR%
copy libgcc_s_dw2-1.dll %TARGET_DIR%
copy QtCore4.dll %TARGET_DIR%
copy QtGui4.dll %TARGET_DIR%
copy QtNetwork4.dll %TARGET_DIR%

copy dlt_viewer.exe %TARGET_DIR%
copy qextserialport.dll %TARGET_DIR%
copy qdlt.dll %TARGET_DIR%

copy plugins\dltviewerplugin.dll %TARGET_DIR%\plugins
copy plugins\nonverboseplugin.dll %TARGET_DIR%\plugins

copy ..\..\DLT-viewer\ReleaseNotes_Viewer.txt %TARGET_DIR%

copy ..\..\DLT-viewer\qdlt\dlt.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\dlt_common.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\dlt_protocol.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\dlt_types.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\dlt_user.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\dlt_user_macros.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\qdlt\qdlt.h %TARGET_DIR%\sdk\include
copy ..\..\DLT-viewer\src\plugininterface.h %TARGET_DIR%\sdk\include

copy libqdlt.a %TARGET_DIR%\sdk\lib
copy libqextserialport.a %TARGET_DIR%\sdk\lib

copy ..\..\DLT-viewer\plugin\dummyviewerplugin %TARGET_DIR%\sdk\src\dummyviewerplugin
copy ..\..\DLT-viewer\plugin\dummydecoderplugin %TARGET_DIR%\sdk\src\dummydecoderplugin
copy ..\..\DLT-viewer\plugin\dummycontrolplugin %TARGET_DIR%\sdk\src\dummycontrolplugin

copy ..\..\DLT-viewer\sdk\BuildPlugins.pro %TARGET_DIR%\sdk\src
copy ..\..\DLT-viewer\sdk\dummydecoderplugin.pro %TARGET_DIR%\sdk\src\dummydecoderplugin
copy ..\..\DLT-viewer\sdk\dummyviewerplugin.pro %TARGET_DIR%\sdk\src\dummyviewerplugin
copy ..\..\DLT-viewer\sdk\dummycontrolplugin.pro %TARGET_DIR%\sdk\src\dummycontrolplugin

cd ..\..\DLT-viewer\sdk