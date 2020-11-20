# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

*-gcc* {
    QMAKE_CFLAGS += -std=gnu99
    QMAKE_CFLAGS += -Wall
    QMAKE_CFLAGS += -Wextra
    #QMAKE_CFLAGS += -pedantic
}

*-g++* {
    QMAKE_CXXFLAGS += -std=gnu++0x
    QMAKE_CXXFLAGS += -Wall
    QMAKE_CXXFLAGS += -Wextra
    #QMAKE_CXXFLAGS += -pedantic
}

CONFIG += console

macx {
    QMAKE_LFLAGS += -Wl,-rpath .
} else {
    QMAKE_LFLAGS += -Wl,-rpath=.
}

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

# Compile time defines
unix:DEFINES += BYTE_ORDER=LITTLE_ENDIAN _TTY_POSIX_ QT_VIEWER
win32:DEFINES += BYTE_ORDER=LITTLE_ENDIAN QT_VIEWER

# Local includes
INCLUDEPATH = . ../qdlt

# Icon for application (The smiley face)
icons.files = icon/org.genivi.DLTViewer.ico
icons_16x16.files = icon/16x16/org.genivi.DLTViewer.png
icons_22x22.files = icon/22x22/org.genivi.DLTViewer.png
icons_24x24.files = icon/24x24/org.genivi.DLTViewer.png
icons_32x32.files = icon/32x32/org.genivi.DLTViewer.png
icons_48x48.files = icon/48x48/org.genivi.DLTViewer.png
icons_256x256.files = icon/256x256/org.genivi.DLTViewer.png
icons_symbolic.files = icon/symbolic/org.genivi.DLTViewer-symbolic.svg

icons.path = $$PREFIX/usr/share/pixmaps
icons_16x16.path = $$PREFIX/usr/share/icons/hicolor/16x16/apps
icons_22x22.path = $$PREFIX/usr/share/icons/hicolor/22x22/apps
icons_24x24.path = $$PREFIX/usr/share/icons/hicolor/24x24/apps
icons_32x32.path = $$PREFIX/usr/share/icons/hicolor/32x32/apps
icons_48x48.path = $$PREFIX/usr/share/icons/hicolor/48x48/apps
icons_256x256.path = $$PREFIX/usr/share/icons/hicolor/256x256/apps
icons_symbolic.path = $$PREFIX/usr/share/icons/hicolor/symbolic/apps

INSTALLS += icons icons_16x16 icons_22x22 icons_24x24 icons_32x32 icons_48x48 icons_256x256 icons_symbolic

# desktop file to show the application in start menu on Linux
# This should work on both KDE and Gnome
desktop.path = $$PREFIX/usr/share/applications
desktop.files = org.genivi.DLTViewer.desktop
INSTALLS += desktop

# Unix header exports
headers.path = $$PREFIX/usr/include/dlt-viewer
headers.files = plugininterface.h
INSTALLS += headers

# Unix executable install path
target.path = $$PREFIX/usr/bin
INSTALLS += target

# Building an app bundle for macx is not so easy, hence disable it
# for development build
macx:CONFIG -= app_bundle

# Library definitions for debug and release builds
CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    QMAKE_CXXFLAGS += -g
    QMAKE_CFLAGS += -g
    QMAKE_LIBDIR += ../debug
    LIBS += -lqdltd
} else {
    DESTDIR = ../release
    QMAKE_LIBDIR += ../release
    LIBS += -lqdlt
    QMAKE_RPATHDIR += ../build/release
}

win32-g++ {
    LIBS += -lws2_32
}

# QT Features to be linked in
QT += core gui network serialport

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
TARGET = dlt-viewer

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
    exporterdialog.cpp \
    dltmsgqueue.cpp \
    dltfileindexerthread.cpp \
    dltfileindexerdefaultfilterthread.cpp \
    mcudpsocket.cpp \

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
    exporterdialog.h \
    dltmsgqueue.h \
    dltfileindexerthread.h \
    dltfileindexerdefaultfilterthread.h \
    mcudpsocket.h \
    regex_search_replace.h

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
    exporterdialog.ui \

# Related files
OTHER_FILES += \
    ../AUTHORS \
    ../build_sdk_windows_qt5.bat \
    ../build_sdk_windows_qt5_MSVC.bat \
    ../INSTALL.txt \
    ../LICENSE.txt \
    ../MPL.txt \
    ../README.md \
    ../ReleaseNotes_Viewer.txt \

# This contains all the button graphics for toolbar
RESOURCES += resource.qrc

# Instructions to compile the icon.
RC_FILE = icon/dlt_viewer.rc

DISTFILES += \
    ../build_qwt_windows_qt5_MSVC.bat
