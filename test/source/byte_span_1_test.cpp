#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/byte_span.hpp"

using lsm::utils::byte_span;
using lsm::utils::byte_view;
using lsm::utils::cbyte_view;
using lsm::utils::detail::byte_like;
using lsm::utils::detail::const_convertible;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

struct trivial_struct {
  int x;
  double y = 1.0;
};

struct non_trivial_struct {
  std::vector<int> data;
};

template <typename First, typename Second>
struct type_pair {
  using a = First;
  using b = Second;
};

// clang-format off
#define BYTE_LIKE_PAIRS \
  (type_pair<char, char>), \
  (type_pair<char, unsigned char>), \
  (type_pair<char, std::byte>), \
  (type_pair<unsigned char, char>), \
  (type_pair<unsigned char, unsigned char>), \
  (type_pair<unsigned char, std::byte>), \
  (type_pair<std::byte, char>), \
  (type_pair<std::byte, unsigned char>), \
  (type_pair<std::byte, std::byte>)
// clang-format on

TEMPLATE_TEST_CASE("const_convertible cross-type checks - should pass",
                   "[concept][template]",
                   BYTE_LIKE_PAIRS) {
  using from = typename TestType::a;
  using to = typename TestType::b;
  STATIC_REQUIRE(const_convertible<from&, to&>);
  STATIC_REQUIRE(const_convertible<from&, const to&>);
  STATIC_REQUIRE(const_convertible<const from&, const to&>);
  STATIC_REQUIRE(const_convertible<from, to>);
  STATIC_REQUIRE(const_convertible<from, const to>);
  STATIC_REQUIRE(const_convertible<const from, const to>);
  STATIC_REQUIRE(const_convertible<from&, to>);
  STATIC_REQUIRE(const_convertible<from&, const to>);
  STATIC_REQUIRE(const_convertible<const from&, const to>);
  STATIC_REQUIRE(const_convertible<from, to&>);
  STATIC_REQUIRE(const_convertible<from, const to&>);
  STATIC_REQUIRE(const_convertible<const from, const to&>);
}

TEST_CASE("byte_span default constructor", "[byte_span]") {
  STATIC_REQUIRE(std::is_default_constructible_v<byte_span<std::byte>>);
  STATIC_REQUIRE(std::is_default_constructible_v<byte_span<const std::byte>>);
  STATIC_REQUIRE(std::is_default_constructible_v<byte_span<std::byte, 0>>);

  STATIC_REQUIRE_FALSE(
      std::is_default_constructible_v<byte_span<std::byte, 10>>);

  auto const span = byte_span<std::byte>{};
  REQUIRE(span.empty());
}

TEMPLATE_TEST_CASE("byte_span construction",
                   "[byte_span][template]",
                   char,
                   unsigned char,
                   std::byte) {
  using t = TestType;
  auto vec = std::vector<t>{t{1}, t{2}, t{3}};
  auto const cvec = std::vector<t>{t{1}, t{2}, t{3}};

  SECTION("from ByteLikeContiguousIterator - iterator and count") {
    auto const span = byte_span{vec.begin(), vec.size()};
    REQUIRE(span.size() == vec.size());
    REQUIRE(span.data()
            == reinterpret_cast<decltype(span)::pointer>(vec.data()));

    auto const cspan = byte_span{cvec.begin(), cvec.size()};
    REQUIRE(cspan.size() == cvec.size());
    REQUIRE(cspan.data()
            == reinterpret_cast<decltype(cspan)::pointer>(cvec.data()));
  }

  SECTION("from ByteLikeContiguousIterator - iterator and sentinel") {
    auto const span = byte_span{vec.begin(), vec.end()};
    REQUIRE(span.size() == vec.size());
    REQUIRE(span.data()
            == reinterpret_cast<decltype(span)::pointer>(vec.data()));

    auto const cspan = byte_span{cvec.begin(), cvec.end()};
    REQUIRE(cspan.size() == cvec.size());
    REQUIRE(cspan.data()
            == reinterpret_cast<decltype(cspan)::pointer>(cvec.data()));
  }
}

TEMPLATE_TEST_CASE("byte_span construction from non-byte types",
                   "[byte_span][template]",
                   int,
                   double,
                   trivial_struct) {
  using t = TestType;
  auto vec = std::vector<t>{t{1}, t{2}, t{3}};
  auto const cvec = std::vector<t>{t{1}, t{2}, t{3}};

  SECTION("from NonByteLikeContiguousIterator - iterator and count") {
    auto const span = byte_span{vec.begin(), vec.size()};
    REQUIRE(span.size() == vec.size() * sizeof(t));
    REQUIRE(span.data()
            == reinterpret_cast<decltype(span)::pointer>(vec.data()));

    auto const cspan = byte_span{cvec.begin(), cvec.size()};
    REQUIRE(cspan.size() == cvec.size() * sizeof(t));
    REQUIRE(cspan.data()
            == reinterpret_cast<decltype(cspan)::pointer>(cvec.data()));
  }

  SECTION("from NonByteLikeContiguousIterator - iterator and sentinel") {
    auto const span = byte_span{vec.begin(), vec.end()};
    REQUIRE(span.size() == vec.size() * sizeof(t));
    REQUIRE(span.data()
            == reinterpret_cast<decltype(span)::pointer>(vec.data()));

    auto const cspan = byte_span{cvec.begin(), cvec.end()};
    REQUIRE(cspan.size() == cvec.size() * sizeof(t));
    REQUIRE(cspan.data()
            == reinterpret_cast<decltype(cspan)::pointer>(cvec.data()));
  }
}

TEST_CASE("byte_span construction from void pointer", "[byte_span]") {
  auto buffer = std::array<std::byte, 8>{};
  void* data = buffer.data();
  const void* const_data = buffer.data();

  STATIC_REQUIRE(
      std::is_constructible_v<byte_span<std::byte>, void*, std::size_t>);
  STATIC_REQUIRE(
      std::is_constructible_v<byte_span<const std::byte>, void*, std::size_t>);
  STATIC_REQUIRE(std::is_constructible_v<byte_span<const std::byte>,
                                         const void*, std::size_t>);

  // const safety
  STATIC_REQUIRE_FALSE(
      std::is_constructible_v<byte_span<std::byte>, const void*, std::size_t>);

  auto span = byte_span<std::byte>{data, buffer.size()};
  auto cspan = byte_span<const std::byte>{data, buffer.size()};
  auto cspan2 = byte_span<const std::byte>{const_data, buffer.size()};

  REQUIRE(span.size() == buffer.size());
  REQUIRE(cspan.size() == buffer.size());
  REQUIRE(cspan2.size() == buffer.size());
  REQUIRE(span.data() == buffer.data());
  REQUIRE(cspan.data() == buffer.data());
  REQUIRE(cspan2.data() == buffer.data());

  // CTAD test
  auto ctad_span = byte_span{data, buffer.size()};
  auto ctad_cspan = byte_span{const_data, buffer.size()};
}

TEMPLATE_TEST_CASE("byte_span construction from byte-like C-style arrays",
                   "[byte_span][template]",
                   char,
                   unsigned char,
                   std::byte) {
  using t = TestType;
  t arr[3] = {t{1}, t{2}, t{3}};              // NOLINT
  const t const_arr[3] = {t{1}, t{2}, t{3}};  // NOLINT

  SECTION("implicit conversion") {
    // Non-const array to non-const span
    const byte_span span = arr;
    REQUIRE(span.size() == sizeof(arr));
    REQUIRE(span.data() == reinterpret_cast<std::byte*>(arr));
    REQUIRE(span.extent == sizeof(arr));

    // Non-const array to const span
    const byte_span const_span = arr;
    REQUIRE(const_span.size() == sizeof(arr));
    REQUIRE(const_span.data() == reinterpret_cast<const std::byte*>(arr));
    REQUIRE(const_span.extent == sizeof(arr));

    // Const array to const span
    const byte_span const_span2 = const_arr;
    REQUIRE(const_span2.size() == sizeof(const_arr));
    REQUIRE(const_span2.data()
            == reinterpret_cast<const std::byte*>(const_arr));
    REQUIRE(const_span2.extent == sizeof(const_arr));
  }

  // Const safety check
  STATIC_REQUIRE_FALSE(
      // NOLINTNEXTLINE
      std::is_constructible_v<byte_span<std::byte>, const t(&)[3]>);
}

TEMPLATE_TEST_CASE("byte_span construction from non-byte-like C-style arrays",
                   "[byte_span][template]",
                   int,
                   double,
                   trivial_struct) {
  using t = TestType;
  t arr[3] = {t{1}, t{2}, t{3}};              // NOLINT
  const t const_arr[3] = {t{1}, t{2}, t{3}};  // NOLINT

  SECTION("explicit conversion required") {
    // Non-const array to non-const span
    auto span = byte_span{arr};  // CTAD
    REQUIRE(span.size() == sizeof(arr));
    REQUIRE(span.data() == reinterpret_cast<std::byte*>(arr));

    // Non-const array to const span
    auto const_span = byte_span<const std::byte>{arr};
    REQUIRE(const_span.size() == sizeof(arr));
    REQUIRE(const_span.data() == reinterpret_cast<const std::byte*>(arr));

    // Const array to const span
    auto const_span2 = byte_span{const_arr};  // CTAD
    REQUIRE(const_span2.size() == sizeof(const_arr));
    REQUIRE(const_span2.data()
            == reinterpret_cast<const std::byte*>(const_arr));
  }

  // Const safety check
  STATIC_REQUIRE_FALSE(std::is_constructible_v<byte_span<std::byte>,
                                               const int(&)[3]>);  // NOLINT

  // Non-trivial type check
  STATIC_REQUIRE_FALSE(
      std::is_constructible_v<byte_span<std::byte>,
                              non_trivial_struct(&)[3]>);  // NOLINT
}

TEMPLATE_TEST_CASE("byte_span construction from std::array - byte-like types",
                   "[byte_span][array][template]",
                   char,
                   unsigned char,
                   std::byte) {
  using t = TestType;
  auto arr = std::array<t, 3>{t{1}, t{2}, t{3}};
  const auto const_arr = std::array<t, 3>{t{1}, t{2}, t{3}};
  auto const_elem_arr = std::array<const t, 3>{t{1}, t{2}, t{3}};
  const auto const_all_arr = std::array<const t, 3>{t{1}, t{2}, t{3}};

  SECTION("non-const array - implicit conversion") {
    const byte_span span = arr;  // implicit CTAD
    REQUIRE(span.size() == sizeof(t) * 3);
    REQUIRE(span.data() == reinterpret_cast<std::byte*>(arr.data()));
    REQUIRE(span.extent == sizeof(t) * 3);

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, std::byte>);
  }

  SECTION("const array - const result") {
    const byte_span span = const_arr;  // implicit CTAD
    REQUIRE(span.size() == sizeof(t) * 3);
    REQUIRE(span.data()
            == reinterpret_cast<const std::byte*>(const_arr.data()));
    REQUIRE(span.extent == sizeof(t) * 3);

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, const std::byte>);
  }

  SECTION("const elements - const result") {
    const byte_span span = const_elem_arr;  // implicit CTAD
    REQUIRE(span.size() == sizeof(t) * 3);
    REQUIRE(span.data()
            == reinterpret_cast<const std::byte*>(const_elem_arr.data()));
    REQUIRE(span.extent == sizeof(t) * 3);

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, const std::byte>);
  }

  SECTION("const array and elements - const result") {
    const byte_span span = const_all_arr;  // implicit CTAD
    REQUIRE(span.size() == sizeof(t) * 3);
    REQUIRE(span.data()
            == reinterpret_cast<const std::byte*>(const_all_arr.data()));
    REQUIRE(span.extent == sizeof(t) * 3);

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, const std::byte>);
  }

  // const safety check
  SECTION("const safety") {
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, const std::array<t, 3>&>);
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>, std::array<const t, 3>&>);
  }
}

TEMPLATE_TEST_CASE("byte_span construction from std::array - non-byte types",
                   "[byte_span][array][template]",
                   int,
                   double,
                   trivial_struct) {
  using t = TestType;
  auto arr = std::array<t, 3>{t{1}, t{2}, t{3}};
  const auto const_arr = std::array<t, 3>{t{1}, t{2}, t{3}};
  auto const_elem_arr = std::array<const t, 3>{t{1}, t{2}, t{3}};
  const auto const_all_arr = std::array<const t, 3>{t{1}, t{2}, t{3}};

  SECTION("non-const int array - explicit conversion required") {
    STATIC_REQUIRE_FALSE(
        std::is_convertible_v<std::array<int, 3>&, byte_span<std::byte>>);

    // explicit conversion required
    auto const span = byte_span{arr};  // CTAD
    REQUIRE(span.size() == sizeof(t) * arr.size());
    REQUIRE(span.data() == reinterpret_cast<std::byte*>(arr.data()));
    REQUIRE(span.extent == sizeof(t) * arr.size());

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, std::byte>);
  }

  SECTION("const int array - const result") {
    auto const span = byte_span{const_arr};  // CTAD
    REQUIRE(span.size() == sizeof(t) * const_arr.size());
    REQUIRE(span.data()
            == reinterpret_cast<const std::byte*>(const_arr.data()));
    REQUIRE(span.extent == sizeof(t) * const_arr.size());

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, const std::byte>);
  }

  SECTION("const elements - const result") {
    auto const span = byte_span{const_elem_arr};  // CTAD
    REQUIRE(span.size() == sizeof(t) * const_elem_arr.size());
    REQUIRE(span.data()
            == reinterpret_cast<const std::byte*>(const_elem_arr.data()));
    REQUIRE(span.extent == sizeof(t) * const_elem_arr.size());

    STATIC_REQUIRE(
        std::is_same_v<typename decltype(span)::element_type, const std::byte>);
  }

  SECTION("non-trivial type check") {
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<byte_span<std::byte>,
                                std::array<non_trivial_struct, 1>&>);
  }

  SECTION("const safety") {
    STATIC_REQUIRE_FALSE(std::is_constructible_v<byte_span<std::byte>,
                                                 const std::array<int, 3>&>);
    STATIC_REQUIRE_FALSE(std::is_constructible_v<byte_span<std::byte>,
                                                 std::array<const int, 3>&>);
  }
}

TEST_CASE("byte_span construction from std::array - size checks",
          "[byte_span][array]") {
  auto arr = std::array<int, 3>{1, 2, 3};

  SECTION("dynamic extent") {
    auto const span = byte_span<std::byte>{arr};
    REQUIRE(span.size() == sizeof(int) * 3);
    STATIC_REQUIRE(span.extent == lsm::utils::dynamic_extent);
  }

  SECTION("static extent") {
    auto const span = byte_span<std::byte, sizeof(int) * 3>{arr};
    REQUIRE(span.size() == sizeof(int) * 3);
    STATIC_REQUIRE(span.extent == sizeof(int) * 3);
  }

  SECTION("invalid static extent") {
    STATIC_REQUIRE_FALSE(
        (std::is_constructible_v<byte_span<std::byte, sizeof(int) * 2>,
                                 std::array<int, 3>&>));
  }

  SECTION("zero extent") {
    auto empty_arr = std::array<int, 0>{};
    auto const span = byte_span{empty_arr};  // CTAD
    REQUIRE(span.extent == 0);
    REQUIRE(span.empty());
  }
}

#pragma GCC diagnostic pop
