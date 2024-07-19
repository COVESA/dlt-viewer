#
TEMPLATE = subdirs
CONFIG   += ordered
SUBDIRS  += qdlt src plugin commander
CONFIG += c++1z

ICON = Project.icns
QMAKE_INFO_PLIST = Info.plist

# Qt doesn't copy over the .icns file ?!
RESOURCE_FILES.files = $$ICON
RESOURCE_FILES.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE_FILES
