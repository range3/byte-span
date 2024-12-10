#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/byte_span.hpp"

// NOLINTBEGIN(misc-const-correctness,clang-analyzer-deadcode.DeadStores)

using lsm::utils::as_bytes;
using lsm::utils::as_span;
using lsm::utils::as_sv;
using lsm::utils::as_writable_bytes;
using lsm::utils::as_writable_span;
using lsm::utils::byte_span;
using lsm::utils::byte_view;
using lsm::utils::cbyte_view;
using lsm::utils::detail::byte_like;
using lsm::utils::detail::const_convertible;
using namespace std::string_view_literals;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

TEMPLATE_TEST_CASE("byte_span from std::span - byte types",
                   "[byte_span][span][template]",
                   char,
                   unsigned char,
                   std::byte) {
  using t = TestType;
  auto vec = std::vector<t>{t{1}, t{2}, t{3}};
  auto s = std::span<t>{vec};
  const auto cs = std::span<const t>{vec};
  const auto css = std::span<const t, 3>{vec};

  SECTION("implicit conversions") {
    // byte-like type allows implicit conversion for dynamic_extent
    const byte_span<std::byte> bs = s;
    const byte_span<const std::byte> cbs = s;
    const byte_span<const std::byte> cbs2 = cs;

    REQUIRE(bs.size() == s.size());
    REQUIRE(bs.data() == reinterpret_cast<std::byte*>(s.data()));
  }

  SECTION("explicit conversions") {
    // static extent requires explicit conversion
    auto bs = byte_span<std::byte, 3>{s};
    auto cbs = byte_span<const std::byte, 3>{s};
    auto cbs2 = byte_span<const std::byte, 3>{cs};

    REQUIRE(bs.size() == s.size());
    REQUIRE(bs.data() == reinterpret_cast<std::byte*>(s.data()));
  }

  SECTION("CTAD") {
    auto bs = byte_span{s};
    auto cbs = byte_span{cs};
    auto cbss = byte_span{css};

    STATIC_REQUIRE(std::is_same_v<decltype(bs), byte_span<std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(cbs), byte_span<const std::byte>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(cbss), byte_span<const std::byte, 3>>);
  }
}

TEMPLATE_TEST_CASE("byte_span from std::span - non-byte types",
                   "[byte_span][span][template]",
                   int,
                   double) {
  using t = TestType;
  auto vec = std::vector<t>{t{1}, t{2}, t{3}};
  auto s = std::span<t>{vec};
  const auto cs = std::span<const t>{vec};

  SECTION("implicit conversions - not allowed for non-byte types") {
    STATIC_REQUIRE_FALSE(
        std::is_convertible_v<std::span<t>, byte_span<std::byte>>);
  }

  SECTION("explicit conversions") {
    // Non-byte types require explicit conversion
    auto bs = byte_span<std::byte>{s};
    auto cbs = byte_span<const std::byte>{s};
    auto cbs2 = byte_span<const std::byte>{cs};

    REQUIRE(bs.size() == sizeof(t) * s.size());
    REQUIRE(bs.data() == reinterpret_cast<std::byte*>(s.data()));
  }

  SECTION("CTAD") {
    // CTAD works but requires explicit conversion
    auto bs = byte_span{s};
    auto cbs = byte_span{cs};

    STATIC_REQUIRE(std::is_same_v<decltype(bs), byte_span<std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(cbs), byte_span<const std::byte>>);
    REQUIRE(bs.size() == sizeof(t) * s.size());
  }

  // const safety check
  STATIC_REQUIRE_FALSE(
      std::is_constructible_v<byte_span<std::byte>, std::span<const t>>);
}

TEST_CASE("byte_span from std::span - static extent specializations",
          "[byte_span][span]") {
  std::array<int, 3> arr{1, 2, 3};
  auto s = std::span<int, 3>{arr};
  const auto cs = std::span<const int, 3>{arr};

  SECTION("dynamic extent") {
    // Explicit conversion required for non-byte types
    auto bs = byte_span<std::byte>{s};
    auto cbs = byte_span<const std::byte>{cs};

    REQUIRE(bs.size() == sizeof(int) * 3);
    REQUIRE(cbs.size() == sizeof(int) * 3);
  }

  SECTION("matching static extent") {
    // Always requires explicit conversion
    auto bs = byte_span<std::byte, sizeof(int) * 3>{s};
    auto cbs = byte_span<const std::byte, sizeof(int) * 3>{cs};

    REQUIRE(bs.size() == sizeof(int) * 3);
    REQUIRE(cbs.size() == sizeof(int) * 3);
  }

  SECTION("CTAD") {
    auto bs = byte_span{s};
    auto cbs = byte_span{cs};

    STATIC_REQUIRE(
        std::is_same_v<decltype(bs), byte_span<std::byte, 3 * sizeof(int)>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(cbs),
                       byte_span<const std::byte, cs.extent * sizeof(int)>>);
    REQUIRE(bs.size() == sizeof(int) * 3);
  }

  SECTION("conversions that should fail") {
    // Mismatched static extent
    STATIC_REQUIRE_FALSE(
        (std::is_constructible_v<byte_span<std::byte, sizeof(int) * 2>,
                                 std::span<int, 3>>));

    // const safety
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, std::span<const int, 3>>);
  }
}

TEST_CASE("byte_span construction from other byte_span", "[byte_span]") {
  std::vector<char> vec{1, 2, 3};
  const byte_span<std::byte> original{std::span{vec}};
  const byte_span<const std::byte> const_original{std::span{vec}};
  const byte_span<std::byte, 3> static_original{std::span{vec}};

  SECTION("implicit conversions - dynamic extent") {
    // non-const to non-const
    const byte_span<std::byte> bs = original;
    // non-const to const
    const byte_span<const std::byte> cbs1 = original;
    // const to const
    const byte_span<const std::byte> cbs2 = const_original;
    // static to dynamic
    const byte_span<std::byte> bs2 = static_original;

    REQUIRE(bs.size() == original.size());
    REQUIRE(bs.data() == original.data());
    REQUIRE(bs2.extent == lsm::utils::dynamic_extent);
  }

  SECTION("explicit conversions - static extent") {
    // dynamic to static extent requires explicit conversion
    auto bs = byte_span<std::byte, 3>{original};
    auto cbs = byte_span<const std::byte, 3>{const_original};

    REQUIRE(bs.size() == original.size());
    REQUIRE(bs.data() == original.data());
    REQUIRE(bs.extent == 3);
    REQUIRE(cbs.extent == 3);
  }

  SECTION("CTAD") {
    // From non-const span
    auto bs1 = byte_span{original};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    // From const span
    auto bs2 = byte_span{const_original};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // From static extent
    auto bs3 = byte_span{static_original};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs3),
                       byte_span<std::byte, static_original.extent>>);
  }
}

TEST_CASE("byte_span construction from string_view",
          "[byte_span][string_view]") {
  SECTION("from non-const string_view") {
    std::string_view sv = "Hello";
    auto bs = byte_span{sv};

    STATIC_REQUIRE(std::is_same_v<decltype(bs), byte_span<const std::byte>>);
    REQUIRE(bs.size() == 5 * sizeof(char));
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }

  SECTION("from const string_view") {
    const std::string_view sv = "Hello";
    auto bs = byte_span{sv};

    STATIC_REQUIRE(std::is_same_v<decltype(bs), byte_span<const std::byte>>);
    REQUIRE(bs.size() == 5 * sizeof(char));
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }

  SECTION("empty string_view") {
    std::string_view sv;
    auto bs = byte_span{sv};

    REQUIRE(bs.empty());
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }

  SECTION("static extent - should assert") {
    std::string_view sv = "Hello";
    auto bs = byte_span<const std::byte, 5>{
        sv};  // expect assertion if sizeof(char) != 1

    REQUIRE(bs.size() == 5);
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }
}

TEMPLATE_TEST_CASE("byte_span construction from various string_view types",
                   "[byte_span][string_view]",
                   char,
                   wchar_t,
                   char8_t,
                   char16_t,
                   char32_t) {
  using t = TestType;
  using sv_t = std::basic_string_view<t>;
  std::array<t, 3> arr = {t{65}, t{66}, t{67}};  // "ABC"
  sv_t sv{arr.data(), arr.size()};

  SECTION("dynamic extent") {
    auto bs = byte_span{sv};

    STATIC_REQUIRE(std::is_same_v<decltype(bs), byte_span<const std::byte>>);
    REQUIRE(bs.size() == 3 * sizeof(t));
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }

  SECTION("static extent") {
    auto bs = byte_span<const std::byte, 3 * sizeof(t)>{sv};

    REQUIRE(bs.size() == 3 * sizeof(t));
    REQUIRE(bs.data() == reinterpret_cast<const std::byte*>(sv.data()));
  }
}

TEST_CASE("byte_span construction from string_view literals",
          "[byte_span][string_view]") {
  SECTION("dynamic extent with various literals") {
    auto bs1 = byte_span{"Hello"sv};
    auto bs2 = byte_span{L"Hello"sv};
    auto bs3 = byte_span{u8"Hello"sv};
    auto bs4 = byte_span{u"Hello"sv};
    auto bs5 = byte_span{U"Hello"sv};

    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<const std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<const std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);
    STATIC_REQUIRE(std::is_same_v<decltype(bs5), byte_span<const std::byte>>);

    REQUIRE(bs1.size() == 5 * sizeof(char));
    REQUIRE(bs2.size() == 5 * sizeof(wchar_t));
    REQUIRE(bs3.size() == 5 * sizeof(char8_t));
    REQUIRE(bs4.size() == 5 * sizeof(char16_t));
    REQUIRE(bs5.size() == 5 * sizeof(char32_t));
  }
}

class buffer {
  static constexpr size_t size = 256;
  std::array<std::byte, size> data_{};
  size_t used_{};

 public:
  auto write(cbyte_view src) -> size_t {
    auto count = std::min(src.size(), size - used_);
    std::copy_n(src.begin(), count, data_.begin() + used_);
    used_ += count;
    return count;
  }

  auto read(byte_view dest) const -> size_t {
    auto count = std::min(dest.size(), used_);
    std::copy_n(data_.begin(), count, dest.data());
    return count;
  }

  auto view() const -> cbyte_view { return {data_.data(), used_}; }
};

// NOLINTNEXTLINE
TEST_CASE("byte_span conversion utilities", "[byte_span][span]") {
  SECTION("byte_span to std::span<(const) std::byte>") {
    std::array<std::byte, 12> data{};
    auto bytes = byte_span{data};

    auto writable = as_writable_bytes(bytes);
    static_assert(std::is_same_v<decltype(writable), std::span<std::byte, 12>>);

    auto readonly = as_bytes(bytes);
    static_assert(
        std::is_same_v<decltype(readonly), std::span<const std::byte, 12>>);

    // Const byte_span can only be converted to const bytes
    const auto const_bytes = byte_span{data};
    auto const_view = as_bytes(const_bytes);
    static_assert(
        std::is_same_v<decltype(const_view), std::span<const std::byte, 12>>);

    // This would not compile:
    // auto invalid = as_writable_bytes(const_bytes);
  }

  SECTION("byte_span to std::span<T>") {
    std::array<int, 3> data = {1, 2, 3};
    auto bytes = byte_span{data};

    auto readonly = as_span<int>(bytes);
    static_assert(std::is_same_v<decltype(readonly), std::span<const int, 3>>);
    REQUIRE(readonly[0] == 1);

    auto writable = as_writable_span<int>(bytes);
    static_assert(std::is_same_v<decltype(writable), std::span<int, 3>>);
    writable[0] = 42;
    REQUIRE(data[0] == 42);

    // Const byte_span can only be converted to const span
    const auto const_bytes = byte_span{data};
    auto const_view = as_span<int>(const_bytes);

    // This would not compile:
    // auto invalid = as_writable_span<int>(const_bytes);
  }

  SECTION("byte_span to string_view") {
    auto text = "Hello"sv;
    auto bytes = byte_span{text.data(), text.size()};

    auto sv = as_sv(bytes);
    REQUIRE(sv == text);
  }

  SECTION("dynamic extent") {
    auto buf = buffer{};

    auto text = "Hello, World!"sv;
    REQUIRE(buf.write(byte_span{text.data(), text.size()}) == text.size());
    REQUIRE(as_sv(buf.view()) == text);

    struct point {
      float x, y;
    };
    auto p = point{1.0F, 2.0F};

    REQUIRE(buf.write(byte_span{&p, 1}) == sizeof(p));
    const auto& result =
        as_value<point>(buf.view().subspan(text.size(), sizeof(p)));
    REQUIRE(result.x == Catch::Approx(p.x));
    REQUIRE(result.y == Catch::Approx(p.y));
  }

  SECTION("static extent") {
    struct alignas(4) small_struct {
      uint32_t value;
    };
    struct alignas(8) large_struct {
      uint64_t value;
    };

    std::array<std::byte, 4> data{};
    auto view = byte_span<std::byte, 4>{data};

    // OK: sizeof(SmallStruct) <= 4
    static_assert(sizeof(small_struct) <= 4);
    const auto& small = as_value<small_struct>(view);

    // Would not compile: sizeof(LargeStruct) > 4
    // auto& large = as_value<LargeStruct>(view);  // Compilation error
  }
}

#pragma GCC diagnostic pop

// NOLINTEND(misc-const-correctness,clang-analyzer-deadcode.DeadStores)
