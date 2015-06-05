# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(dltlogstorageplugin)

# plugin header files
HEADERS  += \
    dltlogstorageconfigcreatorplugin.h \
    logstorageconfigcreatorform.h \
    logstoragefilter.h

# plugin source files
SOURCES += \
    dltlogstorageconfigcreatorplugin.cpp \
    logstorageconfigcreatorform.cpp \
    logstoragefilter.cpp

# plugin forms
FORMS    += \
    logstorageconfigcreatorform.ui
