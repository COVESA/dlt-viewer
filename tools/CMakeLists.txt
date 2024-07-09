add_library(tools
    dltmessagematcher.cpp
    dltmessagematcher.h
)

target_link_libraries(tools PUBLIC qdlt)
target_include_directories(tools PUBLIC .)

add_subdirectory(tests)
