#pragma once

#include <cstddef>

#include <array>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace column {

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

  [[nodiscard]] constexpr std::size_t size() const {
    return (end_ - begin_) / sizeof(T);
  }

  constexpr column_ref_iterator<T> begin() const {
    return column_ref_iterator<T>{begin_};
  }

  constexpr column_ref_iterator<T> end() const {
    return column_ref_iterator<T>{end_};
  }

  constexpr T& operator[](std::size_t index) {
    return *reinterpret_cast<T*>(begin_ + index * sizeof(T));
  }

  constexpr const T& operator[](std::size_t index) const {
    return *reinterpret_cast<T*>(begin_ + index * sizeof(T));
  }
};

template <class TupleLike> class column_vector {
private:
  static constexpr std::size_t num_types{std::tuple_size_v<TupleLike>};

  static constexpr auto t_index_seq = std::make_index_sequence<num_types>{};

  template <std::size_t Index>
  using types = std::tuple_element_t<Index, TupleLike>;

  template <std::size_t... Indices>
  static constexpr auto get_alignments(std::index_sequence<Indices...>) {
    return std::array{alignof(types<Indices>)...};
  }
  static constexpr std::array t_alignments{get_alignments(t_index_seq)};

  template <std::size_t... Indices>
  static constexpr auto get_sizes(std::index_sequence<Indices...>) {
    return std::array{sizeof(types<Indices>)...};
  }
  static constexpr std::array t_sizes{get_sizes(t_index_seq)};

  template <template <class> class pred, std::size_t... I>
  static constexpr bool each_type(std::index_sequence<I...>) {
    return (pred<types<I>>::value && ...);
  }

public:
  static_assert(num_types > 0, "At least one type required.");
  static_assert(each_type<std::is_nothrow_move_constructible>(t_index_seq),
                "All types must be noexcept move constructible!");
  static_assert(each_type<std::is_nothrow_destructible>(t_index_seq),
                "All types must be noexcept destructible!");

  template <std::size_t Index> column_ref<types<Index>> column() {
    static_assert(Index < num_types, "Invalid index to column_vector::column");
    return column_ref<types<Index>>{storage.get() + get_offset<Index>(cap),
                                    storage.get() + get_offset<Index>(cap) +
                                        sz * t_sizes[Index]};
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return sz; }

  [[nodiscard]] constexpr std::size_t capacity() const noexcept { return cap; }

  template <class... Us> void emplace_back(Us&&... values) {
    if (sz == cap) {
      relocate();
    }
    emplace_back_internal(t_index_seq, std::forward<Us>(values)...);
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
    move_content(t_index_seq, new_storage);
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

    using Tp = types<Column>;
    std::byte* src = storage.get();
    for (std::size_t i = 0; i < sz; ++i) {
      Tp* src_it = reinterpret_cast<Tp*>(src + offset + i * t_sizes[Column]);
      void* dst_it = static_cast<void*>(dst + offset + i * t_sizes[Column]);
      new (dst_it) Tp(std::move(*src_it));
      reinterpret_cast<Tp*>(src_it)->~Tp();
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
