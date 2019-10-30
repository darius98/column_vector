cmake_minimum_required(VERSION 3.15)
project(column)

set(CMAKE_CXX_STANDARD 17)

add_library(column_vector INTERFACE)
target_include_directories(column_vector INTERFACE include)

set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(google_benchmark)

add_executable(bm_all
    benchmarks/emplace_back_unaligned.cpp
    benchmarks/emplace_back_aligned.cpp
    benchmarks/iterate_column_unaligned.cpp
    benchmarks/iterate_column_aligned.cpp)
target_link_libraries(bm_all PUBLIC column_vector benchmark_main)
target_compile_definitions(bm_all PUBLIC LINK_ALL_BENCHMARKS)

add_custom_target(compare_to_std_vector
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compare_to_std_vector.sh
        "${CMAKE_CURRENT_SOURCE_DIR}/google_benchmark/tools/compare.py"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(compare_to_std_vector bm_all)
