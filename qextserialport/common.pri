infile(config.pri, QEXTSERIALPORT_LIBRARY, yes): CONFIG += qextserialport-library
qextserialport-library{
infile(config.pri, QEXTSERIALPORT_STATIC, yes): CONFIG += qextserialport-static
}
# Though maybe you have been fimiliar with "TEMPLATE += fakelib" and "TEMPLATE -= fakelib",
# but it don't work when you using "qmake -tp XXX". So I use another variable Here.
SAVE_TEMPLATE = $$TEMPLATE
TEMPLATE = fakelib

CONFIG(debug, debug|release) {
    QEXTSERIALPORT_LIBNAME = qextserialportd
    TEMPLATE = $$SAVE_TEMPLATE
    QEXTSERIALPORT_LIBDIR = ../../debug #$$PWD/lib
}
else {
    QEXTSERIALPORT_LIBNAME = qextserialport
    TEMPLATE = $$SAVE_TEMPLATE
    QEXTSERIALPORT_LIBDIR = ../../release #$$PWD/lib
}
unix:qextserialport-library:!qextserialport-buildlib:QMAKE_RPATHDIR += $$QEXTSERIALPORT_LIBDIR
