# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dltsystemviewerplugin)

# plugin header files
HEADERS += \
    dltsystemviewerplugin.h \
    form.h

# plugin source files
SOURCES += \
    dltsystemviewerplugin.cpp \
    form.cpp

# plugin forms
FORMS += \
    form.ui
