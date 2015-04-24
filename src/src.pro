# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

# Compile time defines
unix:DEFINES += BYTE_ORDER=LITTLE_ENDIAN _TTY_POSIX_ QT_VIEWER
win32:DEFINES += BYTE_ORDER=LITTLE_ENDIAN QT_VIEWER

# Local includes
INCLUDEPATH = . ../qextserialport/src ../qdlt

# Icon for application (The smiley face)
icons.path = /usr/share/pixmaps
icons.files = icon/face-glasses.ico
INSTALLS += icons

# desktop file to show the application in start menu on Linux
# This should work on both KDE and Gnome
desktop.path = /usr/share/applications
desktop.files = dlt_viewer.desktop
INSTALLS += desktop

# Unix header exports
headers.path = /usr/include/dlt-viewer
headers.files = plugininterface.h
INSTALLS += headers

# Unix executable install path
target.path = /usr/bin
INSTALLS += target

# Library definitions for debug and release builds
CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    QMAKE_LIBDIR += ../debug
    LIBS += -lqextserialportd -lqdltd
} else {
    DESTDIR = ../release
    QMAKE_LIBDIR += ../release
    LIBS += -lqextserialport -lqdlt
    QMAKE_RPATHDIR += ../build/release
}

# QT Features to be linked in
QT += core gui network

# Detect QT5 and comply to new Widgets hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    DEFINES += QT5
}

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# Executable name
TARGET = dlt_viewer

# This is an application
TEMPLATE = app

# Compile these sources
SOURCES += main.cpp \
    mainwindow.cpp \
    project.cpp \
    ecudialog.cpp \
    applicationdialog.cpp \
    contextdialog.cpp \
    filterdialog.cpp \
    plugindialog.cpp \
    settingsdialog.cpp \
    injectiondialog.cpp \
    searchdialog.cpp \
    multiplecontextdialog.cpp \
    optmanager.cpp \
    dltsettingsmanager.cpp \
    tablemodel.cpp \
    filtertreewidget.cpp \
    dltfileutils.cpp \
    dltfileindexer.cpp \
    dlttableview.cpp \
    dltexporter.cpp \
    fieldnames.cpp \
    dltuiutils.cpp \
    workingdirectory.cpp \
    jumptodialog.cpp\
    searchtablemodel.cpp \
    pulsebutton.cpp \
    plugintreewidget.cpp \
    exporterdialog.cpp

# Show these headers in the project
HEADERS += mainwindow.h \
    project.h \
    ecudialog.h \
    applicationdialog.h \
    contextdialog.h \
    filterdialog.h \
    plugindialog.h \
    settingsdialog.h \
    injectiondialog.h \
    searchdialog.h \
    version.h \
    multiplecontextdialog.h \
    optmanager.h \
    dltsettingsmanager.h \
    tablemodel.h \
    filtertreewidget.h \
    dltfileutils.h \
    dltfileindexer.h \
    dlttableview.h \
    dltexporter.h \
    fieldnames.h \
    workingdirectory.h \
    dltuiutils.h \
    jumptodialog.h \
    pulsebutton.h \
    searchtablemodel.h \
    plugintreewidget.h \
    exporterdialog.h

# Compile these UI files
FORMS += mainwindow.ui \
    ecudialog.ui \
    applicationdialog.ui \
    contextdialog.ui \
    filterdialog.ui \
    plugindialog.ui \
    settingsdialog.ui \
    injectiondialog.ui \
    searchdialog.ui \
    multiplecontextdialog.ui \
    jumptodialog.ui \
    exporterdialog.ui

# Related files
OTHER_FILES += ../ReleaseNotes_Viewer.txt \
    ../README.txt \
    ../MPL.txt \
    ../LICENSE.txt \
    ../INSTALL.txt \
    ../build_windows_installer.bat \
    ../build_sdk_windows.bat \
    ../build_sdk_msvc_windows64.bat \
    ../build_sdk_msvc_windows.bat \
    ../installer/UninstallLog.nsh \
    ../installer/dlt_installer.nsi \
    ../build_sdk_windows_qt5.bat \
    ../AUTHORS

# This contains all the button graphics for toolbar
RESOURCES += resource.qrc

# Instructions to compile the icon.
RC_FILE = icon/dlt_viewer.rc
