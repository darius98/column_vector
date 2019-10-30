cmake_minimum_required(VERSION 3.15)
project(column)

set(CMAKE_CXX_STANDARD 17)

add_library(column_vector INTERFACE)
target_include_directories(column_vector INTERFACE include)

set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(google_benchmark)

function(add_benchmark name)
    add_executable(bm_${name} benchmarks/${name}.cpp)
    target_link_libraries(bm_${name} column_vector benchmark)
endfunction()

add_benchmark(emplace_back_unaligned)
add_benchmark(emplace_back_large)
add_benchmark(emplace_back_aligned)
add_benchmark(iterate_column_unaligned)
add_benchmark(iterate_column_large)
add_benchmark(iterate_column_aligned)
