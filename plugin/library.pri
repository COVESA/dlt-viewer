# Common project definitions for a library

# Defines
QT                     -= gui
TEMPLATE                = lib

# Include path
INCLUDEPATH +=

# Library path
CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../debug
}
else {
    QMAKE_LIBDIR += ../../release
}

# Libraries
CONFIG(debug, debug|release) {
    LIBS +=
}
else {
    LIBS +=
}

# Destination
CONFIG(debug, debug|release) {
    DESTDIR = ../../debug
}
else {
    DESTDIR = ../../release
}
target.path = $$PREFIX/usr/lib
INSTALLS += target
