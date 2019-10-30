#include <vector>

#include <benchmark/benchmark.h>
#include <column_vector.h>

#include "data.h"

void column_vector_emplace_back_aligned(benchmark::State& state) {
  for (auto _ : state) {
    column::column_vector<int, int, int, int> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3, 4);
    }
  }
}
BENCHMARK(column_vector_emplace_back_aligned) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

void std_vector_emplace_back_aligned(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<Aligned> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3, 4);
    }
  }
}
BENCHMARK(std_vector_emplace_back_aligned) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
