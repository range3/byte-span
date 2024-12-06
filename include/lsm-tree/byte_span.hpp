#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <span>

namespace lsm::utils {

namespace detail {
template <typename T>
concept Byte = std::same_as<std::remove_cv_t<T>, char> ||
               std::same_as<std::remove_cv_t<T>, unsigned char> ||
               std::same_as<std::remove_cv_t<T>, std::byte>;

template <typename It>
concept ByteContiguousIterator = requires {
  requires std::contiguous_iterator<It>;
  requires Byte<std::iter_value_t<It>>;
};

template <typename It>
concept NonByteContiguousIterator = requires {
  requires std::contiguous_iterator<It>;
  requires !Byte<std::iter_value_t<It>>;
  requires std::is_trivially_copyable_v<std::iter_value_t<It>>;
};

}  // namespace detail

template <detail::Byte B, size_t Extent = std::dynamic_extent>
class basic_byte_span {
 public:
  using span_type = std::span<B, Extent>;
  using element_type = typename span_type::element_type;
  using value_type = typename span_type::value_type;
  using size_type = typename span_type::size_type;
  using difference_type = typename span_type::difference_type;
  using pointer = typename span_type::pointer;
  using const_pointer = typename span_type::const_pointer;
  using reference = typename span_type::reference;
  using const_reference = typename span_type::const_reference;
  using iterator = typename span_type::iterator;
  using reverse_iterator = typename span_type::reverse_iterator;
  // using const_iterator = typename span_type::const_iterator;
  // using const_reverse_iterator = typename span_type::const_reverse_iterator;

  static constexpr size_t extent = Extent;

  constexpr basic_byte_span() noexcept
    requires(Extent == std::dynamic_extent || Extent == 0)
      : span_{} {}

  // For ByteContiguousIterator - implicit conversion allowed
  // iterator and size
  template <detail::ByteContiguousIterator It>
    requires(!std::is_const_v<
                std::remove_reference_t<std::iter_reference_t<It>>>) ||
            std::is_const_v<element_type>
  constexpr explicit(Extent != std::dynamic_extent)
      basic_byte_span(It first, size_type count) noexcept
      : span_{reinterpret_cast<pointer>(std::to_address(first)), count} {}

  // iterator and sentinel
  template <detail::ByteContiguousIterator It, std::sized_sentinel_for<It> End>
    requires(((!std::is_const_v<
                  std::remove_reference_t<std::iter_reference_t<It>>>) ||
              std::is_const_v<element_type>) &&
             (!std::is_convertible_v<End, size_type>))
  constexpr explicit(Extent != std::dynamic_extent)
      basic_byte_span(It first, End last) noexcept(noexcept(last - first))
      : span_{reinterpret_cast<pointer>(std::to_address(first)),
              static_cast<size_type>(last - first)} {}

  // For NonByteContiguousIterator - always explicit
  // iterator and size
  template <detail::NonByteContiguousIterator It>
    requires(!std::is_const_v<
                std::remove_reference_t<std::iter_reference_t<It>>>) ||
            std::is_const_v<element_type>
  constexpr explicit basic_byte_span(It first, size_type count) noexcept
      : span_(reinterpret_cast<pointer>(std::to_address(first)),
              count * sizeof(std::iter_value_t<It>)) {}

  // iterator and sentinel
  template <detail::NonByteContiguousIterator It,
            std::sized_sentinel_for<It> End>
    requires((!std::is_const_v<
                 std::remove_reference_t<std::iter_reference_t<It>>>) ||
             std::is_const_v<element_type>) &&
            (!std::is_convertible_v<End, size_type>)
  constexpr explicit basic_byte_span(It first,
                                     End last) noexcept(noexcept(last - first))
      : span_{reinterpret_cast<pointer>(std::to_address(first)),
              static_cast<size_type>(last - first) *
                  sizeof(std::iter_value_t<It>)} {}

  // For void* - always explicit
  // iterator and size
  template <typename VoidType>
    requires std::is_void_v<VoidType> &&
             ((!std::is_const_v<VoidType>) || std::is_const_v<element_type>)
  constexpr explicit basic_byte_span(VoidType* data, size_type size) noexcept
      : span_{reinterpret_cast<pointer>(data), size} {}

  // iterator and sentinel is not provided

  constexpr auto data() const noexcept { return span_.data(); }
  constexpr auto size() const noexcept { return span_.size(); }
  constexpr auto size_bytes() const noexcept { return span_.size_bytes(); }
  constexpr auto empty() const noexcept { return span_.empty(); }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-use-default-member-init,modernize-use-default-member-init)
  span_type span_;
};

using byte_span = basic_byte_span<std::byte>;
using cbyte_span = basic_byte_span<const std::byte>;

}  // namespace lsm::utils
