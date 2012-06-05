TEMPLATE=lib
CONFIG += qt qextserialport-buildlib
# Include .pri file before using "qextserialport-static"
# and after CONFIG += "qextserialport-buildlib"
include(../src/qextserialport.pri)
qextserialport-static:CONFIG += static
else:CONFIG += dll
mac:CONFIG += absolute_library_soname
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all
TARGET = $$QEXTSERIALPORT_LIBNAME
DESTDIR = $$QEXTSERIALPORT_LIBDIR
win32:!qextserialport-static{
    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${QEXTSERIALPORT_LIBNAME}.dll
}
target.path = $$DESTDIR
INSTALLS += target
