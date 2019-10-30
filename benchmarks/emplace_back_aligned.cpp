#include "column_vector_benchmark.h"

void column_vector_emplace_back_aligned(benchmark::State& state) {
  for (auto _ : state) {
    column::column_vector<Aligned> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3, 4);
    }
  }
}
COLUMN_VECTOR_BM(column_vector_emplace_back_aligned); // NOLINT(cert-err58-cpp)

void std_vector_emplace_back_aligned(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<Aligned> v;
    const std::size_t num = state.range(0);
    for (std::size_t i = 0; i < num; ++i) {
      v.emplace_back(1, 2, 3, 4);
    }
  }
}
STD_VECTOR_BM(std_vector_emplace_back_aligned); // NOLINT(cert-err58-cpp)
