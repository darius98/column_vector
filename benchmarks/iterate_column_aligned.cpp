#include <random>
#include <vector>

#include <benchmark/benchmark.h>
#include <column_vector.h>

#include "data.h"

void column_vector_iterate_column_aligned(benchmark::State& state) {
  column::column_vector<int, int, int, int> v;
  std::random_device rd;
  const std::size_t num = state.range(0);
  for (std::size_t i = 0; i < num; ++i) {
    v.emplace_back(std::uniform_int_distribution{0, 10}(rd), 2, 3, 4);
  }
  for (auto _ : state) {
    int sum = 0;
    for (const auto& i : v.column<0>()) {
      sum += i;
    }
    benchmark::DoNotOptimize(sum);
  }
}
BENCHMARK(column_vector_iterate_column_aligned) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

void std_vector_iterate_column_aligned(benchmark::State& state) {
  std::vector<Aligned> v;
  std::random_device rd;
  const std::size_t num = state.range(0);
  for (std::size_t i = 0; i < num; ++i) {
    v.emplace_back(std::uniform_int_distribution{0, 10}(rd), 2, 3, 4);
  }
  for (auto _ : state) {
    int sum = 0;
    for (const auto& i : v) {
      sum += i.x;
    }
    benchmark::DoNotOptimize(sum);
  }
}
BENCHMARK(std_vector_iterate_column_aligned) // NOLINT(cert-err58-cpp)
    ->RangeMultiplier(8)
    ->Range(512, 1u << 21u)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
