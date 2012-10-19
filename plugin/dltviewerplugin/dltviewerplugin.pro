# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dltviewerplugin)

# plugin header files
HEADERS += \
    dltviewerplugin.h \
    form.h

# plugin source files
SOURCES += \
    dltviewerplugin.cpp \
    form.cpp

# plugin forms
FORMS += \
    form.ui
