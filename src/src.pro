unix:DEFINES += BYTE_ORDER=LITTLE_ENDIAN _TTY_POSIX_ QT_VIEWER
win32:DEFINES += BYTE_ORDER=LITTLE_ENDIAN QT_VIEWER
INCLUDEPATH = ../qextserialport/src ../qdlt

icons.path = /usr/share/pixmaps
icons.files = icon/face-glasses.ico
INSTALLS += icons

desktop.path = /usr/share/applications
desktop.files = dlt_viewer.desktop
INSTALLS += desktop

headers.path = /usr/include/dlt-viewer
headers.files = plugininterface.h
INSTALLS += headers

target.path = /usr/bin
INSTALLS += target

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    QMAKE_LIBDIR += ../debug
    LIBS += -lqextserialportd -lqdltd
} else {
    DESTDIR = ../release
    QMAKE_LIBDIR += ../release
    LIBS += -lqextserialport -lqdlt
}

QT += core gui network
OBJECTS_DIR = obj
MOC_DIR = moc

TARGET = dlt_viewer

TEMPLATE = app

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
    commandplugindialog.cpp \
    filtertreewidget.cpp

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
    dlt_common.h \
    dlt_user_shared.h \
    qdlt.h \
    plugininterface.h \
    version.h \
    multiplecontextdialog.h \
    optmanager.h \
    dltsettingsmanager.h \
    tablemodel.h \
    commandplugindialog.h \
    filtertreewidget.h

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
    commandplugindialog.ui

OTHER_FILES += png/edit-clear.png \
    png/document-save-as.png \
    png/document-open.png \
    png/document-new.png \
    png/preferences-desktop.png \
    png/network-transmit-receive.png \
    png/network-offline.png \
    png/edit-cut.png

RESOURCES += resource.qrc

RC_FILE = icon/dlt_viewer.rc
