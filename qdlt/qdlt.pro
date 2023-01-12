
PROJECT                 = qdlt
TEMPLATE                = lib

CONFIG += c++11
DEFINES += QDLT_LIBRARY
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
    QMAKE_CXXFLAGS += -DPLUGIN_INSTALLATION_PATH=\\\"$$PREFIX/usr/share/dlt-viewer/plugins\\\"
    #QMAKE_CXXFLAGS += -pedantic
}

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

unix:DEFINES += BYTE_ORDER=LITTLE_ENDIAN _TTY_POSIX_ QT_VIEWER
win32:DEFINES += BYTE_ORDER=LITTLE_ENDIAN QT_VIEWER

headers.path = $$PREFIX/usr/include/qdlt
headers.files = *.h

INSTALLS += headers

target.path = $$PREFIX/$$[QT_INSTALL_LIBS]
INSTALLS += target

CONFIG                 += warn_on qt
QT                     += network
QT                     += gui
QT                     += serialport

# Detect QT5 and comply to new Widgets hierarchy
greaterThan(QT_VERSION, 4.8.4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    win32:DEFINES += QT5_QT6_COMPAT
}

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

INCLUDEPATH = . ../src

SOURCES +=  \
    dlt_common.c \
    qdltipconnection.cpp \
    qdltmessagedecoder.cpp \
    qdlttcpconnection.cpp \
    qdltudpconnection.cpp \
    qdltserialconnection.cpp \
    qdltmsg.cpp \
    qdltfilter.cpp \
    qdltfile.cpp \
    qdltcontrol.cpp \
    qdltconnection.cpp \
    qdltbase.cpp \
    qdltargument.cpp \
    qdltfilterlist.cpp \
    qdltfilterindex.cpp \
    qdltdefaultfilter.cpp \
    qdltpluginmanager.cpp \
    qdltplugin.cpp \
    qdltoptmanager.cpp \
    qdltsegmentedmsg.cpp \
    qdltsettingsmanager.cpp \

HEADERS += qdlt.h \
    export_rules.h \
    dlt_common.h \
    dlt_user.h \
    qdltipconnection.h \
    qdltmessagedecoder.h \
    qdlttcpconnection.h \
    qdltudpconnection.h \
    qdltserialconnection.h \
    qdltmsg.h \
    qdltfilter.h \
    qdltfile.h \
    qdltcontrol.h \
    qdltconnection.h \
    qdltbase.h \
    qdltargument.h \
    qdltfilterlist.h \
    qdltfilterindex.h \
    qdltdefaultfilter.h \
    plugininterface.h \
    qdltpluginmanager.h \
    qdltplugin.h \
    dlt_types.h \
    dlt_protocol.h \
    qdltoptmanager.h \
    qdltsegmentedmsg.h \
    qdltsettingsmanager.h \

unix:VERSION            = 1.0.0

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    TARGET  = qdltd
    QMAKE_LIBDIR += ../debug
}
else {
    DESTDIR = ../release
    TARGET  = qdlt
    QMAKE_LIBDIR += ../release
}
