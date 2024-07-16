# Copyright (C) 2016, Jack S. Smith
#
# This file is part of COVESA DLT-Commander project.
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

add_executable(dlt-commander
    main.cpp
    optmanager.cpp)

target_link_libraries(dlt-commander
    qdlt
    ${QT_PREFIX}::Core
    ${QT_PREFIX}::Network
    ${QT_PREFIX}::SerialPort)

if(CMAKE_COMPILER_IS_GNUCXX)
    # https://stackoverflow.com/questions/45329372/ubuntu-recognizes-executable-as-shared-library-and-wont-run-it-by-clicking
    # https://forum.juce.com/t/cmake-executable-build-shows-up-as-shared-library-on-linux-mint/45503/6
    target_link_options(dlt-commander PRIVATE "-no-pie")
endif()

set_target_properties(dlt-commander PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    INSTALL_RPATH "$ORIGIN/../lib;$<$<BOOL:${DLT_USE_QT_RPATH}>:${DLT_QT_LIB_DIR}>")

    install(TARGETS dlt-commander
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
    # Underscore for NSIS compatibility https://gitlab.kitware.com/cmake/cmake/-/issues/19982
    COMPONENT dlt_cmd)

if(WIN32)
    set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH "${DLT_EXECUTABLE_INSTALLATION_PATH}")
else()
    set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH ".")
endif()

include(cmake/${CMAKE_SYSTEM_NAME}.cmake OPTIONAL)
