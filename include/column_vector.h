#pragma once

#include <cstddef>

#include <array>
#include <memory>
#include <type_traits>
#include <utility>

namespace column {

namespace mp {

template <std::size_t Index, class... Args> struct nth_type_impl;
template <class A, class... Args> struct nth_type_impl<0, A, Args...> {
  using type = A;
};
template <std::size_t Index, class A, class... Args>
struct nth_type_impl<Index, A, Args...> {
  using type = typename nth_type_impl<Index - 1, Args...>::type;
};
template <std::size_t Index, class... Args>
using nth_type = typename nth_type_impl<Index, Args...>::type;

} // namespace mp

template <class T> struct column_ref_iterator {
  std::byte* pos_;

  constexpr T& operator*() const noexcept {
    return *reinterpret_cast<T*>(pos_);
  }

  constexpr T* operator->() const noexcept { return base(); }

  constexpr column_ref_iterator& operator++() noexcept {
    pos_ += sizeof(T);
    return *this;
  }

  constexpr column_ref_iterator operator++(int) noexcept {
    column_ref_iterator it{pos_};
    ++(*this);
    return it;
  }

  constexpr column_ref_iterator& operator--() noexcept {
    pos_ -= sizeof(T);
    return *this;
  }

  constexpr column_ref_iterator operator--(int) noexcept {
    column_ref_iterator it{pos_};
    --(*this);
    return it;
  }

  constexpr column_ref_iterator operator+(std::ptrdiff_t n) const noexcept {
    return column_ref_iterator{pos_ + n * sizeof(T)};
  }

  constexpr column_ref_iterator& operator+=(std::ptrdiff_t n) const noexcept {
    pos_ += n * sizeof(T);
    return *this;
  }

  constexpr column_ref_iterator operator-(std::ptrdiff_t n) const noexcept {
    return column_ref_iterator{pos_ - n * sizeof(T)};
  }

  constexpr column_ref_iterator& operator-=(std::ptrdiff_t n) const noexcept {
    pos_ -= n * sizeof(T);
    return *this;
  }

  constexpr T* base() const noexcept { return reinterpret_cast<T*>(pos_); }

  friend constexpr bool operator==(column_ref_iterator<T> a,
                                   column_ref_iterator<T> b) noexcept {
    return a.pos_ == b.pos_;
  }

  friend constexpr bool operator!=(column_ref_iterator<T> a,
                                   column_ref_iterator<T> b) noexcept {
    return a.pos_ != b.pos_;
  }

  friend constexpr bool operator<=(column_ref_iterator<T> a,
                                   column_ref_iterator<T> b) noexcept {
    return a.pos_ <= b.pos_;
  }

  friend constexpr bool operator>=(column_ref_iterator<T> a,
                                   column_ref_iterator<T> b) noexcept {
    return a.pos_ >= b.pos_;
  }

  friend constexpr bool operator<(column_ref_iterator<T> a,
                                  column_ref_iterator<T> b) noexcept {
    return a.pos_ < b.pos_;
  }

  friend constexpr bool operator>(column_ref_iterator<T> a,
                                  column_ref_iterator<T> b) noexcept {
    return a.pos_ > b.pos_;
  }

  friend constexpr std::ptrdiff_t operator-(column_ref_iterator<T> a,
                                            column_ref_iterator<T> b) noexcept {
    return (a.pos_ - b.pos_) / sizeof(T);
  }

  friend constexpr column_ref_iterator<T>
  operator+(std::ptrdiff_t a, column_ref_iterator<T> b) noexcept {
    return b + a;
  }
};

template <class T> struct column_ref {
  std::byte* begin_;
  std::byte* end_;

  [[nodiscard]] std::size_t size() const { return (end_ - begin_) / sizeof(T); }

  column_ref_iterator<T> begin() const {
    return column_ref_iterator<T>{begin_};
  }

  column_ref_iterator<T> end() const { return column_ref_iterator<T>{end_}; }

  T& operator[](std::size_t index) {
    return *reinterpret_cast<T*>(begin_ + index * sizeof(T));
  }

  const T& operator[](std::size_t index) const {
    return *reinterpret_cast<T*>(begin_ + index * sizeof(T));
  }
};

template <class... Ts> class column_vector {
private:
  template <std::size_t Index> using types = mp::nth_type<Index, Ts...>;
  static constexpr std::size_t num_types{sizeof...(Ts)};
  static constexpr std::array t_alignments{alignof(Ts)...};
  static constexpr std::array t_sizes{sizeof(Ts)...};
  static constexpr auto t_index_sequence = std::index_sequence_for<Ts...>{};

public:
  static_assert(num_types > 0, "At least one type required.");
  static_assert((std::is_nothrow_move_constructible_v<Ts> && ...),
                "All types must be noexcept move constructible!");
  static_assert((std::is_nothrow_destructible_v<Ts> && ...),
                "All types must be noexcept destructible!");

  template <std::size_t Index> column_ref<types<Index>> column() {
    static_assert(Index < num_types, "Invalid index to column_vector::column");
    return column_ref<types<Index>>{storage.get() + get_offset<Index>(cap),
                                    storage.get() + get_offset<Index>(cap) +
                                        sz * t_sizes[Index]};
  }

  [[nodiscard]] std::size_t size() const noexcept { return sz; }

  [[nodiscard]] std::size_t capacity() const noexcept { return cap; }

  template <class... Us> void emplace_back(Us&&... values) {
    static_assert(
        sizeof...(Us) == num_types,
        "Invalid number of arguments to column_vector::emplace_back!");
    static_assert(
        (std::is_nothrow_constructible_v<Ts, decltype(values)> && ...),
        "Invalid arguments to column_vector::emplace_back!");

    if (sz == cap) {
      relocate();
    }
    emplace_back_internal(t_index_sequence, std::forward<Us>(values)...);
    ++sz;
  }

private:
  template <std::size_t... Is, class... Us>
  void emplace_back_internal(std::index_sequence<Is...>, Us&&... values) {
    std::size_t offset = 0;
    (emplace_back_column<Is>(offset, std::forward<Us>(values)), ...);
  }

  template <std::size_t Column, class U>
  void emplace_back_column(std::size_t& offset, U&& value) {
    offset = (offset + t_alignments[Column] - 1) / t_alignments[Column] *
             t_alignments[Column];

    new (static_cast<void*>(storage.get() + offset + sz * t_sizes[Column]))
        types<Column>(std::forward<U>(value));

    offset += cap * t_sizes[Column];
  }

  void relocate() {
    const auto new_capacity = cap == 0 ? 4 : cap * 2;
    const auto new_storage = new (std::align_val_t{t_alignments[0]})
        std::byte[get_aligned_size(new_capacity)];
    move_content(t_index_sequence, new_storage);
    storage.reset(new_storage);
    cap = new_capacity;
  }

  template <std::size_t... Is>
  void move_content(std::index_sequence<Is...>, std::byte* destination) {
    std::size_t offset = 0;
    (move_column<Is>(offset, destination), ...);
  }

  template <std::size_t Column>
  void move_column(std::size_t& offset, std::byte* dst) {
    offset = (offset + t_alignments[Column] - 1) / t_alignments[Column] *
             t_alignments[Column];

    using T = types<Column>;
    std::byte* src = storage.get();
    for (std::size_t i = 0; i < sz; ++i) {
      T* src_it = reinterpret_cast<T*>(src + offset + i * t_sizes[Column]);
      void* dst_it = static_cast<void*>(dst + offset + i * t_sizes[Column]);
      new (dst_it) T(std::move(*src_it));
      reinterpret_cast<T*>(src_it)->~T();
    }

    offset += cap * t_sizes[Column];
  }

  template <std::size_t Column> std::size_t get_offset(std::size_t capacity) {
    std::size_t offset = 0;
    for (std::size_t i = 0; i < Column; ++i) {
      offset =
          (offset + t_alignments[i] - 1) / t_alignments[i] * t_alignments[i];
      offset += capacity * t_sizes[i];
    }
    return offset;
  }

  std::size_t get_aligned_size(std::size_t capacity) {
    return get_offset<num_types - 1>(capacity) +
           capacity * t_sizes[num_types - 1];
  }

  std::size_t sz{0};
  std::size_t cap{0};
  std::unique_ptr<std::byte[]> storage{nullptr};
};

} // namespace column
