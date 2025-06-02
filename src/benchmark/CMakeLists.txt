find_package(benchmark REQUIRED)

add_executable(bench_dltfileindexer bench_dltfileindexer.cpp)
target_link_libraries(bench_dltfileindexer PRIVATE benchmark::benchmark dlt_common qdlt)

file(COPY testfile.dlt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ )
