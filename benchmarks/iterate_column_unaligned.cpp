#include "column_vector_benchmark.h"

void column_vector_iterate_column_unaligned(benchmark::State& state) {
  column::column_vector<Unaligned> v;
  std::random_device rd;
  const std::size_t num = state.range(0);
  for (std::size_t i = 0; i < num; ++i) {
    v.emplace_back(std::uniform_int_distribution{0, 10}(rd), 2, 3);
  }
  for (auto _ : state) {
    int sum = 0;
    for (const auto& i : v.column<0>()) {
      sum += i;
    }
    benchmark::DoNotOptimize(sum);
  }
}
COLUMN_VECTOR_BM(
    column_vector_iterate_column_unaligned); // NOLINT(cert-err58-cpp)

void std_vector_iterate_column_unaligned(benchmark::State& state) {
  std::vector<Unaligned> v;
  std::random_device rd;
  const std::size_t num = state.range(0);
  for (std::size_t i = 0; i < num; ++i) {
    v.emplace_back(std::uniform_int_distribution{0, 10}(rd), 2, 3);
  }
  for (auto _ : state) {
    int sum = 0;
    for (const auto& i : v) {
      sum += i.x;
    }
    benchmark::DoNotOptimize(sum);
  }
}
STD_VECTOR_BM(std_vector_iterate_column_unaligned); // NOLINT(cert-err58-cpp)
