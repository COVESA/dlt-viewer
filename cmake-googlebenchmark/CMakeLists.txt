# CMakeList.txt : CMake project for cmake-googlebenchmark
# Version here as per current version of Visual Studio 2022.
# Replace with appropriate version for your set up:
cmake_minimum_required (VERSION 3.15)

project ("cmake-googlebenchmark"
DESCRIPTION "A VS project which uses googlebenchmark for tests"
LANGUAGES CXX)

# Some predefined warnings which may be generally useful,
# not required for the project to work.
if (MSVC)
  add_compile_options(/W4 /w14640)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source to this project's executable.
add_executable (cmake-googlebenchmark "cmake-googlebenchmark.cpp" "cmake-googlebenchmark.h")
# Remember if you want to generate a separate benchmark (not in the sources above),
# to add a different executable as google benchmark defines its own main()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET cmake-googlebenchmark PROPERTY CXX_STANDARD 20)
endif()

include(FetchContent)
# Replace the version here with actual version needed:
FetchContent_Declare(googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.0
)
# Suppress internal tests of googlebenchmark:
set(BENCHMARK_ENABLE_TESTING off)
FetchContent_MakeAvailable(
  googlebenchmark
)

# Include googlebenchmark /include/ directory:
include_directories("${googlebenchmark_SOURCE_DIR}/include")
# Link the project target with googlebenchmark:
target_link_libraries(cmake-googlebenchmark benchmark::benchmark)

# TODO: Add tests and install targets
