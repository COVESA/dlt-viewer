# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dummydecoderplugin)

# plugin header files
HEADERS += \
    dummydecoderplugin.h

# plugin source files
SOURCES += \
    dummydecoderplugin.cpp

# plugin forms
FORMS +=
