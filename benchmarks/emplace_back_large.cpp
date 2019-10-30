#include <vector>

#include <benchmark/benchmark.h>
#include <column_vector.h>

#include "data.h"

void column_vector_emplace_back_large(benchmark::State& state) {
  for (auto _ : state) {
    column::column_vector<long long, long long, long long, long long, long long,
                          long long, long long, long long, long long, long long,
                          long long, long long, long long, long long, long long,
                          long long, long long, long long, long long, long long,
                          long long, long long, long long, long long, long long,
                          long long, long long, long long, long long, long long,
                          long long, long long>
        v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013);
    }
  }
}
BENCHMARK(column_vector_emplace_back_large) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

void std_vector_emplace_back_large(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<Large> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013, 666013, 666013, 666013,
                     666013, 666013, 666013, 666013);
    }
  }
}
BENCHMARK(std_vector_emplace_back_large) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
