#pragma once

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace lsm::utils {

template <typename T>
concept Byte = std::same_as<std::remove_cv_t<T>, char> ||
               std::same_as<std::remove_cv_t<T>, unsigned char> ||
               std::same_as<std::remove_cv_t<T>, std::byte>;

template <Byte B>
class basic_byte_view {
 public:
  using value_type = B;
  using const_value_type = std::add_const_t<value_type>;
  using reference = std::add_lvalue_reference_t<value_type>;
  using const_reference = std::add_lvalue_reference_t<const_value_type>;
  using pointer = std::add_pointer_t<value_type>;
  using const_pointer = std::add_pointer_t<const_value_type>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using index_type = std::size_t;

  constexpr basic_byte_view(pointer data, index_type size) noexcept
      : data_(data), size_(size) {}

  constexpr void swap(basic_byte_view& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }

  constexpr auto data() const noexcept -> pointer { return data_; }
  constexpr auto size() const noexcept -> index_type { return size_; }
  constexpr auto empty() const noexcept -> bool { return size_ == 0U; }

  constexpr auto operator[](index_type index) const noexcept -> reference {
    return data_[index];
  }
  auto at(index_type index) const -> reference {
    if (index >= size_) {
      throw std::out_of_range("byte view access out of range");
    }
    return data_[index];
  }
  constexpr auto front() const noexcept -> reference { return *data_; }
  constexpr auto back() const noexcept -> reference {
    return *(data_ + size_ - 1);
  }

  // iterator
  constexpr auto begin() const noexcept -> iterator { return data_; }
  constexpr auto end() const noexcept -> iterator { return data_ + size_; }
  constexpr auto cbegin() const noexcept -> const_iterator { return data_; }
  constexpr auto cend() const noexcept -> const_iterator {
    return data_ + size_;
  }
  constexpr auto rbegin() const noexcept -> reverse_iterator {
    return reverse_iterator(end());
  }
  constexpr auto rend() const noexcept -> reverse_iterator {
    return reverse_iterator(begin());
  }
  constexpr auto crbegin() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }
  constexpr auto crend() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(begin());
  }

 private:
  pointer data_;
  index_type size_;
};

// Type aliases
using byte_view = basic_byte_view<std::byte>;
using cbyte_view = basic_byte_view<const std::byte>;

template <Byte B>
constexpr void swap(basic_byte_view<B>& left,
                    basic_byte_view<B>& right) noexcept {
  left.swap(right);
}

}  // namespace lsm::utils
