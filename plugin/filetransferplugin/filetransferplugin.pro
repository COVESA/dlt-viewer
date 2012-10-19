# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(filetransferplugin)

# plugin header files
HEADERS += \
    filetransferplugin.h \
    form.h \
    file.h \
    imagepreviewdialog.h \
    textviewdialog.h \
    globals.h \
    configuration.h

# plugin source files
SOURCES += \
    filetransferplugin.cpp \
    form.cpp \
    file.cpp \
    imagepreviewdialog.cpp \
    textviewdialog.cpp \
    configuration.cpp

# plugin forms
FORMS += \
    form.ui \
    imagepreviewdialog.ui \
    textviewdialog.ui

