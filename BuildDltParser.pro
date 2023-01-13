#
CXXFLAGS +=-Wunused

TEMPLATE = subdirs
CONFIG   += ordered
SUBDIRS  += parser
CONFIG += c++11
