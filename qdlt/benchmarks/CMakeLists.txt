find_package(automotive-dlt REQUIRED)

add_executable(indexBenchmark
    index_benchmark.cpp
)

target_link_libraries(indexBenchmark PRIVATE
    benchmark::benchmark
    Qt6::Core
    Genivi::dlt
)
