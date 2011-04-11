#
TEMPLATE = subdirs
CONFIG   += ordered
SUBDIRS  += qextserialport qdlt src plugin



# Generate make targets for doxygen documentation
docsrc.target = doc_src
docsrc.commands = doxygen ./src/doxygen.cfg;
docsrc.depends =

docqdlt.target = doc_qdlt
docqdlt.commands = doxygen ./qdlt/doxygen.cfg;
docqdlt.depends =

docqextserialport.target = doc_qextserialport
docqextserialport.commands = doxygen ./qextserialport/doxygen.cfg;
docqextserialport.depends =

QMAKE_EXTRA_TARGETS += docsrc docqdlt docqextserialport
