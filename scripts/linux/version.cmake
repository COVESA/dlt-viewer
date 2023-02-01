if(WIN32)
    return()
endif()

# Will initialize needed CMAKE_ variables.
project(GenerateVersion CXX)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(NOT LINUX AND NOT APPLE)
    return()
endif()

set(QT5_MIN_VERSION_REQ "5.5.1")
set(QT6_MIN_VERSION_REQ "6.2.0")

# try to find QT6
find_package(Qt6 "6" COMPONENTS Core Network PrintSupport SerialPort Widgets)
if(Qt6_FOUND)
    set(QT_PREFIX Qt6)
    message(STATUS "Found Qt6 version: ${Qt6Core_VERSION}")
    if(${QT_PREFIX}Core_VERSION VERSION_LESS ${QT6_MIN_VERSION_REQ})
        # Presumably Qt6Core implies all dependent libs too
        message(FATAL_ERROR "Due to SerialPort QT6 minimum version required: ${QT6_MIN_VERSION_REQ}")
    endif()
else()
    find_package(Qt5 "5" REQUIRED COMPONENTS Core Network PrintSupport SerialPort Widgets)
    if(Qt5_FOUND)
        set(QT_PREFIX Qt5)
        message(STATUS "Found Qt5 version: ${Qt5Core_VERSION}")
        if(${QT_PREFIX}Core_VERSION VERSION_LESS ${QT5_MIN_VERSION_REQ})
            # Presumably Qt5Core implies all dependent libs too
            message(FATAL_ERROR "QT5 minimum version required: ${QT5_MIN_VERSION_REQ}")
        endif()
    endif()
endif()

set(DLT_QT_VERSION "${${QT_PREFIX}Core_VERSION}" CACHE STRING "DLT_QT_VERSION")
get_target_property(DLT_QT_LIBRARY_PATH ${QT_PREFIX}::Core LOCATION)
get_filename_component(DLT_QT_LIB_DIR ${DLT_QT_LIBRARY_PATH} DIRECTORY)

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count --no-merges HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_PATCH_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/parse_version.sh" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" PACKAGE_MAJOR_VERSION)
execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/parse_version.sh" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" PACKAGE_MAJOR_VERSION
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux"
    OUTPUT_VARIABLE DLT_PROJECT_VERSION_MAJOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE RESULT
)
if(RESULT AND NOT RESULT EQUAL 0)
    message(SEND_ERROR "Failure: ${RESULT}")
else()
    message(RESULT "Success.")
endif()

execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/parse_version.sh" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" PACKAGE_MINOR_VERSION
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux"
    OUTPUT_VARIABLE DLT_PROJECT_VERSION_MINOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/parse_version.sh" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" PACKAGE_PATCH_LEVEL
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux"
    OUTPUT_VARIABLE DLT_PROJECT_VERSION_PATCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(DLT_VERSION_SUFFIX "STABLE-qt${DLT_QT_VERSION}-r${GIT_PATCH_VERSION}_${CMAKE_CXX_LIBRARY_ARCHITECTURE}_${CMAKE_CXX_COMPILER_VERSION}")
