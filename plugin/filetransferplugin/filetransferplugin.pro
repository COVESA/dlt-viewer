TEMPLATE  = lib

CONFIG   += plugin

target.path = /usr/share/dlt-viewer/plugins
INSTALLS += target

CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/plugins
    QMAKE_LIBDIR += ../../debug
    LIBS += -lqdltd
}
else {
    DESTDIR = ../../release/plugins
    QMAKE_LIBDIR += ../../release
    LIBS += -lqdlt
}

TARGET = $$qtLibraryTarget(filetransferplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../src \
            ../../qdlt

# Project files
HEADERS += filetransferplugin.h \
    form.h \
    file.h \
    imagepreviewdialog.h \
    textviewdialog.h \
    globals.h \
    configuration.h

SOURCES += filetransferplugin.cpp \
    form.cpp \
    file.cpp \
    imagepreviewdialog.cpp \
    textviewdialog.cpp \
    configuration.cpp

FORMS += form.ui \
    imagepreviewdialog.ui \
    textviewdialog.ui
