# Copyright (C) 2016, Jack S. Smith
# 
# This file is part of COVESA DLT-Viewer project.
#   
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.
#
# List of changes:
# 01.Oct.2016, Jack Smith <jack.smith@elektrobit.com>, Original Author
#

set (CMAKE_AUTOUIC ON)

MESSAGE("  Searching for QWT !")
FIND_PATH(QWT_INCLUDE_DIR NAMES qwt.h PATHS
  /usr/include
  /usr/local/include
  "$ENV{LIB_DIR}/include" 
  "$ENV{INCLUDE}" 
  PATH_SUFFIXES qwt-qt5 qwt
  )

FIND_LIBRARY(QWT_LIBRARY NAMES qwt qwt5 qwt-qt5 PATHS 
  /usr/lib
  /usr/local/lib
  "$ENV{LIB_DIR}/lib" 
  "$ENV{LIB}/lib" 
  )

IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
  SET(QWT_FOUND TRUE)
ENDIF (QWT_INCLUDE_DIR AND QWT_LIBRARY)


IF (QWT_FOUND)
  IF (NOT QWT_FIND_QUIETLY)
    MESSAGE(STATUS "Found QWT lib: ${QWT_LIBRARY}")
    MESSAGE(STATUS "Found QWT include: ${QWT_INCLUDE_DIR}")
  ENDIF (NOT QWT_FIND_QUIETLY)
ELSE (QWT_FOUND)
  IF (QWT_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find QWT")
  ENDIF (QWT_FIND_REQUIRED)
ENDIF (QWT_FOUND)  


# cmake 2.8.12 doesn't have AUTOUIC ??
QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

add_library(speedplugin MODULE speedplugin.cpp
                               form.cpp
                               ${UI_HEADERS})

target_include_directories(speedplugin PUBLIC ${QWT_INCLUDE_DIR})
target_link_libraries(speedplugin qdlt Qt5::Widgets ${QWT_LIBRARY})
install(TARGETS speedplugin DESTINATION deploy/plugins)
