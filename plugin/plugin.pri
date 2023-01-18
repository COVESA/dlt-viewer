# Common project definitions for a DLT Viewer plugin

# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)


*-gcc* {
    QMAKE_CFLAGS += -std=gnu99
    QMAKE_CFLAGS += -Wall
    QMAKE_CFLAGS += -Wextra
    # Limit symbol visibility to avoid symbol clashes between different
    # plugins
    QMAKE_CXXFLAGS += -fvisibility=hidden
    #QMAKE_CFLAGS += -pedantic
}

*-g++* {
    QMAKE_CXXFLAGS += -std=gnu++0x
    QMAKE_CXXFLAGS += -Wall
    QMAKE_CXXFLAGS += -Wextra
    # Limit symbol visibility to avoid symbol clashes between different
    # plugins
    QMAKE_CXXFLAGS += -fvisibility=hidden
    #QMAKE_CXXFLAGS += -pedantic
}

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

# Defines
DEFINES  += QT_VIEWER

# This is a library
TEMPLATE  = lib

# ...of type plugin
CONFIG   += plugin

# QWT
# CONFIG  += qwt

# QWT_DIR = $$(QWT_DIR)
# !isEmpty(QWT_DIR) {
#     QWT_INSTALL_PREFIX = $$QWT_DIR
# } else {
#     QWT_INSTALL_PREFIX = C:\\Qwt-6.1.3
# }


# Used QT features
QT += core gui network

# Detect QT5 and comply to new Widget hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    win32:DEFINES += QT5_QT6_COMPAT
}



# Include path
INCLUDEPATH += ../../src \
            ../../qdlt \
            /usr/include/qdlt
# QWT
# win32:INCLUDEPATH += $$QWT_INSTALL_PREFIX\\include

# Library path
CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../debug
}
else {
    QMAKE_LIBDIR += ../../release
}

# Libraries
# Qwt Library
# win32:QMAKE_LIBDIR += $$QWT_INSTALL_PREFIX\\lib

CONFIG(debug, debug|release) {
    LIBS += -lqdltd
}
else {
    LIBS += -lqdlt
}

# Destination
CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/plugins
}
else {
    DESTDIR = ../../release/plugins
}
target.path = $$PREFIX/usr/share/dlt-viewer/plugins
INSTALLS += target
