# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dummycommandplugin)

# plugin header files
HEADERS += \
    dummycommandplugin.h \
    dummywaitthread.h

# plugin source files
SOURCES += \
    dummycommandplugin.cpp \
    dummywaitthread.cpp

# plugin forms
FORMS +=

