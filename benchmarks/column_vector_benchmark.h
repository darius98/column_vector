#pragma once

#include <random>
#include <tuple>
#include <vector>

#include <benchmark/benchmark.h>

#include <column_vector.h>

struct Aligned {
  int x;
  int y;
  int z;
  int t;

  Aligned(int x, int y, int z, int t) : x(x), y(y), z(z), t(t) {}
};
template <> struct std::tuple_size<Aligned> {
  static constexpr std::size_t value{4};
};
template <std::size_t N> struct std::tuple_element<N, Aligned> {
  using type = int;
};

struct Unaligned {
  int x;
  double y;
  char z;

  Unaligned(int x, double y, char z) : x(x), y(y), z(z) {}
};
template <> struct std::tuple_size<Unaligned> {
  static constexpr std::size_t value{3};
};
template <> struct std::tuple_element<0, Unaligned> { using type = int; };
template <> struct std::tuple_element<1, Unaligned> { using type = double; };
template <> struct std::tuple_element<2, Unaligned> { using type = char; };

#define COLUMN_VECTOR_BM(func)                                                 \
  BENCHMARK(func)                                                              \
      ->RangeMultiplier(8)                                                     \
      ->Range(512, 1u << 21u)

#define STD_VECTOR_BM(func)                                                    \
  BENCHMARK(func)                                                              \
      ->RangeMultiplier(8)                                                     \
      ->Range(512, 1u << 21u)
