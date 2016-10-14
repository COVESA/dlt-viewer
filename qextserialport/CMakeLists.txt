# Copyright (C) 2016, Jack S. Smith
#
# This file is part of GENIVI DLT-Viewer project.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
#
# List of changes:
# 01.Oct.2016, Jack Smith <jack.smith@elektrobit.com>, Original Author
#

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND QEXT_SERIALPORT_SOURCES src/qextserialport.cpp
                                    src/qextserialenumerator.cpp)

# Process conditional here
# I suspect there's a better way to do this.
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    list(APPEND QEXT_SERIALPORT_SOURCES src/qextserialport_unix.cpp
                                        src/qextserialenumerator_unix.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)  # OSX
    list(APPEND QEXT_SERIALPORT_SOURCES src/qextserialenumerator_osx.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    list(APPEND QEXT_SERIALPORT_SOURCES src/qextserialport_win.cpp
                                        src/qextserialenumerator_win.cpp)
endif()

add_library (qextserialport ${QEXT_SERIALPORT_SOURCES})
# Make sure the compiler can find include files for our qextserialport library
# when other libraries or executables link to qextserialport
target_include_directories (qextserialport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Refactoring GUI components out of this is a good idea
qt5_use_modules(qextserialport Core)
#qt5_use_modules(qextserialport Widgets)

install(TARGETS qextserialport DESTINATION deploy)
