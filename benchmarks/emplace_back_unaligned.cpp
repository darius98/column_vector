#include "column_vector_benchmark.h"

void column_vector_emplace_back_unaligned(benchmark::State& state) {
  for (auto _ : state) {
    column::column_vector<Unaligned> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3);
    }
  }
}
COLUMN_VECTOR_BM(
    column_vector_emplace_back_unaligned); // NOLINT(cert-err58-cpp)

void std_vector_emplace_back_unaligned(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<Unaligned> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3);
    }
  }
}
STD_VECTOR_BM(std_vector_emplace_back_unaligned); // NOLINT(cert-err58-cpp)
