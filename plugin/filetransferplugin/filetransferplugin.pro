TEMPLATE  = lib

CONFIG   += plugin

# QT += network

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
    filemodel.h \
    file.h \
    imagepreviewdialog.h \
    textviewdialog.h \
    globals.h

SOURCES += filetransferplugin.cpp \
    form.cpp \
    filemodel.cpp \
    file.cpp \
    imagepreviewdialog.cpp \
    textviewdialog.cpp

FORMS += \
    form.ui \
    imagepreviewdialog.ui \
    textviewdialog.ui

OTHER_FILES += list-add.png\
                list-remove.png\
            edit-redo.png\
            edit-undo.png\
        document-save-as.png
RESOURCES += \
    filetransferpluginresources.qrc
