#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/byte_span.hpp"

using lsm::utils::byte_span;
using lsm::utils::byte_view;
using lsm::utils::cbyte_view;

struct trivial_struct {
  int x;
  double y;
};

// NOLINTBEGIN(misc-const-correctness)

TEMPLATE_TEST_CASE("byte_span construction from ranges - byte-like types",
                   "[byte_span][range]",
                   char,
                   unsigned char,
                   std::byte) {
  using t = TestType;

  SECTION("from non-const range") {
    std::vector<t> vec{t{1}, t{2}, t{3}};

    // Dynamic extent - implicit conversion allowed for byte-like types
    const byte_span bs1 = vec;

    REQUIRE(bs1.size() == vec.size());
    REQUIRE(bs1.data() == reinterpret_cast<std::byte*>(vec.data()));

    // Static extent - requires explicit conversion
    auto bs2 = byte_span<std::byte, 3>{vec};

    REQUIRE(bs2.size() == 3);
    REQUIRE(bs2.data() == reinterpret_cast<std::byte*>(vec.data()));
  }

  SECTION("from const range") {
    const std::vector<t> vec{t{1}, t{2}, t{3}};

    // Dynamic extent with const data
    const byte_span<const std::byte> bs1 = vec;

    REQUIRE(bs1.size() == vec.size());
    REQUIRE(bs1.data() == reinterpret_cast<const std::byte*>(vec.data()));

    // Static extent with const data
    auto bs2 = byte_span<const std::byte, 3>{vec};

    REQUIRE(bs2.size() == 3);
    REQUIRE(bs2.data() == reinterpret_cast<const std::byte*>(vec.data()));
  }

  SECTION("const safety checks") {
    const std::vector<t> vec{t{1}, t{2}, t{3}};

    // Cannot create non-const byte_span from const range
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, decltype(vec)>);
  }
}

TEMPLATE_TEST_CASE("byte_span construction from ranges - non-byte types",
                   "[byte_span][range]",
                   int,
                   double,
                   trivial_struct) {
  using t = TestType;

  SECTION("from non-const range") {
    std::vector<t> vec(3);

    // Dynamic extent - requires explicit conversion for non-byte types
    auto bs1 = byte_span{vec};  // CTAD

    REQUIRE(bs1.size() == sizeof(t) * vec.size());
    REQUIRE(bs1.data() == reinterpret_cast<std::byte*>(vec.data()));

    // Static extent
    auto bs2 = byte_span<std::byte, sizeof(t) * 3>{vec};

    REQUIRE(bs2.size() == sizeof(t) * 3);
    REQUIRE(bs2.data() == reinterpret_cast<std::byte*>(vec.data()));
  }

  SECTION("from const range") {
    const std::vector<t> vec(3);

    // Dynamic extent with const data
    auto bs1 = byte_span{vec};  // CTAD

    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<const std::byte>>);
    REQUIRE(bs1.size() == sizeof(t) * vec.size());

    // Static extent with const data
    auto bs2 = byte_span<const std::byte, sizeof(t) * 3>{vec};

    REQUIRE(bs2.size() == sizeof(t) * 3);
    REQUIRE(bs2.data() == reinterpret_cast<const std::byte*>(vec.data()));
  }

  SECTION("const safety checks") {
    const std::vector<t> vec(3);

    // Cannot create non-const byte_span from const range
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, decltype(vec)>);
  }

  SECTION("empty range") {
    std::vector<t> vec;

    auto bs = byte_span{vec};
    REQUIRE(bs.empty());
    REQUIRE(bs.size() == 0);
  }
}

TEST_CASE("byte_span construction from ranges - error cases",
          "[byte_span][range]") {
  struct non_trivial {
    std::string str;  // non-trivially copyable
    int x{};
  };

  SECTION("non-trivially copyable type") {
    std::vector<non_trivial> vec(3);

    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, decltype(vec)>);
  }

  SECTION("invalid static extent") {
    std::vector<int> vec(3);

    STATIC_REQUIRE_FALSE(
        (std::is_constructible_v<byte_span<std::byte, sizeof(int) * 2>,
                                 decltype(vec)>));
  }
}

// NOLINTEND(misc-const-correctness)
