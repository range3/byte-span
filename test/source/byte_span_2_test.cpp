#include <array>
#include <cstddef>
// #include <cstdint>
#include <span>
#include <type_traits>
// #include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
// #include <catch2/generators/catch_generators_all.hpp>

#include "lsm-tree/byte_span.hpp"

using lsm::utils::byte_span;
using lsm::utils::byte_view;
using lsm::utils::cbyte_view;
using lsm::utils::detail::byte_like;
using lsm::utils::detail::const_convertible;

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

#pragma GCC diagnostic pop
