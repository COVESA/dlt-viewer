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

add_executable(dlt-parser main.cpp
    mainwindow.cpp
    mainwindow.ui
    qdltparser.cpp)

target_link_libraries(dlt-parser ${QT_PREFIX}::Widgets -lstdc++)

set_target_properties(dlt-parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    INSTALL_RPATH "$ORIGIN/../lib;$<$<BOOL:${DLT_USE_QT_RPATH}>:${DLT_QT_LIB_DIR}>")

    if(CMAKE_COMPILER_IS_GNUCXX)
    # https://stackoverflow.com/questions/45329372/ubuntu-recognizes-executable-as-shared-library-and-wont-run-it-by-clicking
    # https://forum.juce.com/t/cmake-executable-build-shows-up-as-shared-library-on-linux-mint/45503/6
    target_link_options(dlt-parser PRIVATE "-no-pie")
endif()

install(TARGETS dlt-parser
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
    # Underscore for NSIS compatibility https://gitlab.kitware.com/cmake/cmake/-/issues/19982
    COMPONENT dlt_parser)


if(DLT_INSTALL_SDK)
    install(FILES
        ../ReleaseNotes_Parser.txt
        DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
        COMPONENT dlt_parser)
endif()
