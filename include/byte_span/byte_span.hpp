#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>

namespace range3 {

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

template <typename T>
using void_pointer_for =
    std::conditional_t<std::is_const_v<T>, const void*, void*>;

template <typename To, typename From>
constexpr auto pointer_cast(From* p) noexcept -> To* {
  return static_cast<To*>(static_cast<void_pointer_for<From>>(p));
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
#if __cplusplus > 202002L
  using const_iterator = typename span_type::const_iterator;
  using const_reverse_iterator = typename span_type::const_reverse_iterator;
#endif

  static constexpr size_t extent = Extent;

  constexpr byte_span(const byte_span&) noexcept = default;
  constexpr auto operator=(const byte_span&) noexcept -> byte_span& = default;
  constexpr byte_span(byte_span&&) noexcept = default;
  constexpr auto operator=(byte_span&&) noexcept -> byte_span& = default;
  ~byte_span() noexcept = default;

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
      : span_{detail::pointer_cast<element_type>(std::to_address(first)),
              detail::calculate_size<std::iter_value_t<It>>(
                  static_cast<size_type>(last - first))} {}

  // From iterator and count
  template <std::contiguous_iterator It>
    requires std::is_trivially_copyable_v<std::iter_value_t<It>>
          && detail::const_convertible<std::iter_reference_t<It>, element_type&>
  constexpr explicit(Extent != dynamic_extent
                     || !detail::byte_like<std::iter_value_t<It>>)
      byte_span(It first, size_type count) noexcept
      : span_{detail::pointer_cast<element_type>(std::to_address(first)),
              detail::calculate_size<std::iter_value_t<It>>(count)} {}

  // From void* - always explicit
  // void* and size
  template <typename VoidType>
    requires std::is_void_v<VoidType>
          && detail::const_convertible<VoidType, element_type>
  constexpr explicit byte_span(VoidType* data, size_type size) noexcept
      : span_{detail::pointer_cast<element_type>(data), size} {}

  // From c-style arrays
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(T (&arr)[ArrayExtent]) noexcept
      : span_{detail::pointer_cast<element_type>(std::data(arr)),
              detail::calculate_size<T>(ArrayExtent)} {}

  // From std::array
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(std::array<T, ArrayExtent>& arr) noexcept
      : span_{detail::pointer_cast<element_type>(arr.data()),
              detail::calculate_size<T>(ArrayExtent)} {}

  // From const std::array
  template <typename T, size_t ArrayExtent>
    requires std::is_trivially_copyable_v<T>
          && (Extent == dynamic_extent || Extent == ArrayExtent * sizeof(T))
          && detail::const_convertible<const T, element_type>
  constexpr explicit(!detail::byte_like<T>)
      // NOLINTNEXTLINE
      byte_span(const std::array<T, ArrayExtent>& arr) noexcept
      : span_{detail::pointer_cast<element_type>(arr.data()),
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
      : span_{detail::pointer_cast<element_type>(std::ranges::data(range)),
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
      : span_{detail::pointer_cast<element_type>(s.data()),
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

  [[nodiscard]]
  constexpr auto data() const noexcept {
    return span_.data();
  }
  [[nodiscard]]
  constexpr auto size() const noexcept {
    return span_.size();
  }
  [[nodiscard]]
  constexpr auto size_bytes() const noexcept {
    return span_.size_bytes();
  }
  [[nodiscard]]
  constexpr auto empty() const noexcept {
    return span_.empty();
  }
  [[nodiscard]]
  constexpr auto front() const noexcept -> reference {
    assert(!empty());
    return span_.front();
  }

  [[nodiscard]]
  constexpr auto back() const noexcept -> reference {
    assert(!empty());
    return span_.back();
  }

  [[nodiscard]]
  constexpr auto operator[](size_type idx) const noexcept -> reference {
    assert(idx < size());
    return span_[idx];
  }

  [[nodiscard]]
  constexpr auto begin() const noexcept -> iterator {
    return span_.begin();
  }

  [[nodiscard]]
  constexpr auto end() const noexcept -> iterator {
    return span_.end();
  }

  [[nodiscard]]
  constexpr auto rbegin() const noexcept -> reverse_iterator {
    return span_.rbegin();
  }

  [[nodiscard]]
  constexpr auto rend() const noexcept -> reverse_iterator {
    return span_.rend();
  }

#if __cplusplus > 202002L
  // C++23
  [[nodiscard]]
  constexpr auto cbegin() const noexcept -> const_iterator {
    return span_.cbegin();
  }

  [[nodiscard]]
  constexpr auto cend() const noexcept -> const_iterator {
    return span_.cend();
  }

  [[nodiscard]]
  constexpr auto crbegin() const noexcept -> const_reverse_iterator {
    return span_.crbegin();
  }

  [[nodiscard]]
  constexpr auto crend() const noexcept -> const_reverse_iterator {
    return span_.crend();
  }
#endif

  template <size_t Count>
  [[nodiscard]]
  constexpr auto first() const noexcept {
    auto s = span_.template first<Count>();
    return byte_span<element_type, Count>{s};
  }

  [[nodiscard]]
  constexpr auto first(size_type count) const noexcept {
    return byte_span{span_.first(count)};
  }

  template <size_t Count>
  [[nodiscard]]
  constexpr auto last() const noexcept {
    auto s = span_.template last<Count>();
    return byte_span<element_type, Count>{s};
  }

  [[nodiscard]]
  constexpr auto last(size_type count) const noexcept {
    return byte_span{span_.last(count)};
  }

  template <size_t Offset, size_t Count = dynamic_extent>
  [[nodiscard]]
  constexpr auto subspan() const noexcept {
    auto s = span_.template subspan<Offset, Count>();
    return byte_span{s};
  }

  [[nodiscard]]
  constexpr auto subspan(size_type offset,
                         size_type count = dynamic_extent) const noexcept {
    return byte_span{span_.subspan(offset, count)};
  }

  constexpr void swap(byte_span& other) noexcept {
    std::swap(span_, other.span_);
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-use-default-member-init,modernize-use-default-member-init)
  span_type span_;
};

// non-member functions
template <typename B, size_t Extent>
constexpr void swap(byte_span<B, Extent>& lhs,
                    byte_span<B, Extent>& rhs) noexcept {
  lhs.swap(rhs);
}

template <typename B, size_t N>
constexpr auto as_sv(byte_span<B, N> bytes) noexcept -> std::string_view {
  return {detail::pointer_cast<const char>(bytes.data()), bytes.size()};
}

template <typename T, typename B, size_t N>
  requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
            && (!std::is_void_v<T>) && (N == dynamic_extent || N >= sizeof(T))
constexpr auto as_value(byte_span<B, N> bytes) noexcept -> const T& {
  assert(bytes.size() >= sizeof(T));
  return *std::launder(detail::pointer_cast<const T>(bytes.data()));
}

// byte_span -> std::span<std::byte>
template <typename B, size_t N>
  requires(!std::is_const_v<std::remove_reference_t<B>>)
constexpr auto as_writable_bytes(byte_span<B, N> bytes) noexcept
    -> std::span<std::byte, N> {
  return std::span<std::byte, N>{bytes.data(), bytes.size()};
}

// byte_span -> std::span<const std::byte>
template <typename B, size_t N>
constexpr auto as_bytes(byte_span<B, N> bytes) noexcept
    -> std::span<const std::byte, N> {
  return std::span<const std::byte, N>{bytes.data(), bytes.size()};
}

// byte_span -> std::span<T> (writable)
template <typename T, typename B, size_t N>
  requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
            && (!std::is_const_v<std::remove_reference_t<B>>)
            && (!std::is_const_v<std::remove_reference_t<T>>)
            && (N == dynamic_extent || (N % sizeof(T) == 0))
constexpr auto as_writable_span(byte_span<B, N> bytes) noexcept
    -> std::span<T, N == dynamic_extent ? dynamic_extent : N / sizeof(T)> {
  assert(bytes.size() % sizeof(T) == 0);
  return std::span<T, (N == dynamic_extent ? dynamic_extent : N / sizeof(T))>{
      std::launder(detail::pointer_cast<T>(bytes.data())),
      bytes.size() / sizeof(T)};
}

// byte_span -> std::span<const T>
template <typename T, typename B, size_t N>
  requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
            && (N == dynamic_extent || (N % sizeof(T) == 0))
constexpr auto as_span(byte_span<B, N> bytes) noexcept
    -> std::span<const T,
                 N == dynamic_extent ? dynamic_extent : N / sizeof(T)> {
  assert(bytes.size() % sizeof(T) == 0);
  return std::span<const T,
                   (N == dynamic_extent ? dynamic_extent : N / sizeof(T))>{
      std::launder(detail::pointer_cast<const T>(bytes.data())),
      bytes.size() / sizeof(T)};
}

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

}  // namespace range3

// Opt-in to borrowed_range concept
template <typename T, std::size_t N>
constexpr bool std::ranges::enable_borrowed_range<range3::byte_span<T, N>> =
    true;

// Opt-in to view concept
template <typename B, size_t N>
constexpr bool std::ranges::enable_view<range3::byte_span<B, N>> = true;
