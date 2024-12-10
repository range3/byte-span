#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>

namespace lsm::utils {

using std::dynamic_extent;

template <typename B, size_t Extent>
class byte_span;

namespace detail {

template <typename T>
concept byte_like = std::same_as<std::remove_cv_t<T>, char>
                 || std::same_as<std::remove_cv_t<T>, unsigned char>
                 || std::same_as<std::remove_cv_t<T>, std::byte>;

template <typename From, typename To>
concept const_convertible = std::is_const_v<std::remove_reference_t<To>>
                         || !std::is_const_v<std::remove_reference_t<From>>;

template <typename Range>
concept byte_range = requires {
  requires std::ranges::contiguous_range<Range>;
  requires std::ranges::sized_range<Range>;
  requires byte_like<std::ranges::range_value_t<Range>>;
};

template <typename Range>
concept non_byte_range = requires {
  requires std::ranges::contiguous_range<Range>;
  requires std::ranges::sized_range<Range>;
  requires !byte_like<std::ranges::range_value_t<Range>>;
  requires std::is_trivially_copyable_v<std::ranges::range_value_t<Range>>;
};

template <typename Range, typename ElementType>
concept const_safe_range =
    std::is_const_v<ElementType>
    || ((!std::is_const_v<
            std::remove_reference_t<std::ranges::range_value_t<Range>>>)
        && (!std::is_const_v<std::remove_reference_t<Range>>)
        && std::ranges::borrowed_range<Range>);

template <typename T>
concept is_byte_span = requires {
  typename T::element_type;
  { T::extent } -> std::convertible_to<std::size_t>;
} && std::same_as<T, byte_span<typename T::element_type, T::extent>>;

template <typename T>
concept is_std_span = requires {
  typename T::element_type;
  { T::extent } -> std::convertible_to<std::size_t>;
} && std::same_as<T, std::span<typename T::element_type, T::extent>>;

template <typename T>
concept is_std_array =
    requires {
      typename T::value_type;
      std::tuple_size<T>::value;
    }
    && std::same_as<
        T,
        std::array<typename T::value_type, std::tuple_size<T>::value>>;

template <typename T>
constexpr auto calculate_size(size_t count) noexcept -> size_t {
  if constexpr (byte_like<T>) {
    return count;
  } else {
    return count * sizeof(T);
  }
}

}  // namespace detail

template <typename B, size_t Extent = dynamic_extent>
class byte_span {
  static_assert(std::same_as<std::remove_cv_t<B>, std::byte>,
                "byte_span can only be instantiated with std::byte");

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

  constexpr byte_span() noexcept
    requires(Extent == dynamic_extent || Extent == 0)
      : span_{} {}

  // From iterator and sentinel
  template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
    requires std::is_trivially_copyable_v<std::iter_value_t<It>>
          && detail::const_convertible<std::iter_reference_t<It>, element_type&>
  constexpr explicit(Extent != dynamic_extent
                     || !detail::byte_like<std::iter_value_t<It>>)
      byte_span(It first, End last) noexcept(noexcept(last - first))
      : span_{reinterpret_cast<pointer>(std::to_address(first)),
              detail::calculate_size<std::iter_value_t<It>>(
                  static_cast<size_type>(last - first))} {}

  // From iterator and count
  template <std::contiguous_iterator It>
    requires std::is_trivially_copyable_v<std::iter_value_t<It>>
          && detail::const_convertible<std::iter_reference_t<It>, element_type&>
  constexpr explicit(Extent != dynamic_extent
                     || !detail::byte_like<std::iter_value_t<It>>)
      byte_span(It first, size_type count) noexcept
      : span_{reinterpret_cast<pointer>(std::to_address(first)),
              detail::calculate_size<std::iter_value_t<It>>(count)} {}

  // From void* - always explicit
  // void* and size
  template <typename VoidType>
    requires std::is_void_v<VoidType>
          && detail::const_convertible<VoidType, element_type>
  constexpr explicit byte_span(VoidType* data, size_type size) noexcept
      : span_{reinterpret_cast<pointer>(data), size} {}

  // From c-style arrays
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(T (&arr)[ArrayExtent]) noexcept
      : span_{reinterpret_cast<pointer>(arr),
              detail::calculate_size<T>(ArrayExtent)} {}

  // From std::array
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(std::array<T, ArrayExtent>& arr) noexcept
      : span_{reinterpret_cast<pointer>(arr.data()),
              detail::calculate_size<T>(ArrayExtent)} {}

  // From const std::array
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<const T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(const std::array<T, ArrayExtent>& arr) noexcept
      : span_{reinterpret_cast<pointer>(arr.data()),
              detail::calculate_size<T>(ArrayExtent)} {}

  // From Ranges
  template <typename Range>
    requires(!detail::is_std_span<std::remove_cvref_t<Range>>)
         && (!detail::is_byte_span<std::remove_cvref_t<Range>>)
         && (!detail::is_std_array<std::remove_cvref_t<Range>>)
         && (!std::is_array_v<std::remove_cvref_t<Range>>)
         && std::ranges::contiguous_range<Range>
         && std::ranges::sized_range<Range>
         && (std::ranges::borrowed_range<Range>
             || std::is_const_v<element_type>)
         && (detail::const_convertible<std::ranges::range_reference_t<Range>,
                                       element_type&>)
         && std::is_trivially_copyable_v<std::ranges::range_value_t<Range>>
  constexpr explicit(Extent != dynamic_extent
                     || !detail::byte_like<std::ranges::range_value_t<Range>>)
      // NOLINTNEXTLINE
      byte_span(Range&& range) noexcept(noexcept(std::ranges::data(range))
                                        && noexcept(std::ranges::size(range)))
      : span_{reinterpret_cast<pointer>(std::ranges::data(range)),
              detail::calculate_size<std::ranges::range_value_t<Range>>(
                  std::ranges::size(range))} {
    if constexpr (Extent != dynamic_extent) {
      auto const expected_size =
          detail::calculate_size<std::ranges::range_value_t<Range>>(
              std::ranges::size(range));
      assert(expected_size == Extent);
    }
  }

  // From std::span
  template <typename OtherElementType, size_t OtherExtent>
    requires std::is_trivially_copyable_v<OtherElementType>
          && (Extent == dynamic_extent || OtherExtent == dynamic_extent
              || Extent == OtherExtent * sizeof(OtherElementType))
          && detail::const_convertible<OtherElementType, element_type>
  constexpr explicit(!detail::byte_like<OtherElementType>
                     || Extent != dynamic_extent)
      // NOLINTNEXTLINE
      byte_span(const std::span<OtherElementType, OtherExtent>& s) noexcept
      : span_{reinterpret_cast<pointer>(s.data()),
              detail::calculate_size<OtherElementType>(s.size())} {
    if constexpr (Extent != dynamic_extent) {
      assert(detail::calculate_size<OtherElementType>(s.size()) == Extent);
    }
  }

  // From byte_span
  template <typename OtherByte, size_t OtherExtent>
    requires(Extent == dynamic_extent || OtherExtent == dynamic_extent
             || Extent == OtherExtent)
         && detail::const_convertible<OtherByte, element_type>
  constexpr explicit(Extent != dynamic_extent && OtherExtent == dynamic_extent)
      // NOLINTNEXTLINE
      byte_span(const byte_span<OtherByte, OtherExtent>& other) noexcept
      : span_{other.data(), other.size()} {
    if constexpr (Extent != dynamic_extent) {
      assert(other.size() == Extent);
    }
  }

  constexpr auto data() const noexcept { return span_.data(); }
  constexpr auto size() const noexcept { return span_.size(); }
  constexpr auto size_bytes() const noexcept { return span_.size_bytes(); }
  constexpr auto empty() const noexcept { return span_.empty(); }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-use-default-member-init,modernize-use-default-member-init)
  span_type span_;
};

// deduction guides
// From iterator and count
template <std::contiguous_iterator It>
byte_span(It, size_t)
    -> byte_span<std::conditional_t<
        std::is_const_v<std::remove_reference_t<std::iter_reference_t<It>>>,
        const std::byte,
        std::byte>>;

// From iterator and sentinel
template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
byte_span(It, End)
    -> byte_span<std::conditional_t<
        std::is_const_v<std::remove_reference_t<std::iter_reference_t<It>>>,
        const std::byte,
        std::byte>>;

// From void* and size
template <typename VoidType>
  requires std::is_void_v<VoidType>
byte_span(VoidType*, size_t)
    -> byte_span<
        std::conditional_t<std::is_const_v<std::remove_reference_t<VoidType>>,
                           const std::byte,
                           std::byte>>;
// From c-style arrays
template <typename T, size_t ArrayExtent>
byte_span(T (&)[ArrayExtent])  // NOLINT
    ->byte_span<std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
                                   const std::byte,
                                   std::byte>,
                ArrayExtent * sizeof(T)>;

// From std::array
template <typename T, size_t ArrayExtent>
byte_span(std::array<T, ArrayExtent>&)
    -> byte_span<std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
                                    const std::byte,
                                    std::byte>,
                 ArrayExtent * sizeof(T)>;

template <typename T, size_t ArrayExtent>
byte_span(const std::array<T, ArrayExtent>&)
    -> byte_span<const std::byte, ArrayExtent * sizeof(T)>;

// From std::span
template <typename ElementType, size_t Extent>
byte_span(std::span<ElementType, Extent>)
    -> byte_span<std::conditional_t<std::is_const_v<ElementType>,
                                    const std::byte,
                                    std::byte>,
                 Extent == dynamic_extent ? dynamic_extent
                                          : Extent * sizeof(ElementType)>;

// From contiguous_range
template <std::ranges::contiguous_range Range>
byte_span(Range&&)
    -> byte_span<std::conditional_t<std::is_const_v<std::remove_reference_t<
                                        std::ranges::range_reference_t<Range>>>,
                                    const std::byte,
                                    std::byte>>;

// type aliases
using byte_view = byte_span<std::byte>;
using cbyte_view = byte_span<const std::byte>;

}  // namespace lsm::utils
