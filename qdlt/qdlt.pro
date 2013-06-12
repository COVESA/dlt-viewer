
PROJECT                 = qdlt
TEMPLATE                = lib

DEFINES += QDLT_LIBRARY

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

unix:DEFINES += BYTE_ORDER=LITTLE_ENDIAN _TTY_POSIX_ QT_VIEWER
win32:DEFINES += BYTE_ORDER=LITTLE_ENDIAN QT_VIEWER

headers.path = /usr/include/qdlt
headers.files = *.h

INSTALLS += headers

target.path = /usr/lib
INSTALLS += target

CONFIG                 += warn_on qt
QT                     += network
QT                     += gui

OBJECTS_DIR             = build/obj
MOC_DIR                 = build/moc

INCLUDEPATH = ../qextserialport/src ../src

SOURCES +=  \
            dlt_common.c \
    qdlttcpconnection.cpp \
    qdltserialconenction.cpp \
    qdltmsg.cpp \
    qdltfilter.cpp \
    qdltfile.cpp \
    qdltcontrol.cpp \
    qdltconnection.cpp \
    qdltbase.cpp \
    qdltargument.cpp \
    qdltfilterlist.cpp \
    qdltfilterindex.cpp


HEADERS += qdlt.h \
           export_rules.h \
           dlt_common.h \
           dlt_user.h \
    qdlttcpconnection.h \
    qdltserialconnection.h \
    qdltmsg.h \
    qdltfilter.h \
    qdltfile.h \
    qdltcontrol.h \
    qdltconnection.h \
    qdltbase.h \
    qdltargument.h \
    qdltfilterlist.h \
    qdltfilterindex.h

unix:VERSION            = 1.0.0

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    TARGET  = qdltd
    QMAKE_LIBDIR += ../debug
    LIBS += -lqextserialportd
}
else {
    DESTDIR = ../release
    TARGET  = qdlt
    QMAKE_LIBDIR += ../release
    LIBS += -lqextserialport
}
