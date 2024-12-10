#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/byte_span.hpp"

using lsm::utils::byte_span;

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("byte_span CTAD tests", "[byte_span][ctad]") {
  // Common test data setup
  std::vector<char> byte_vec{'a', 'b', 'c'};
  const std::vector<char> const_byte_vec{'a', 'b', 'c'};
  std::vector<int> non_byte_vec{1, 2, 3};
  const std::vector<int> const_non_byte_vec{1, 2, 3};

  char byte_arr[3] = {'a', 'b', 'c'};              // NOLINT
  const char const_byte_arr[3] = {'a', 'b', 'c'};  // NOLINT
  int non_byte_arr[3] = {1, 2, 3};                 // NOLINT
  const int const_non_byte_arr[3] = {1, 2, 3};     // NOLINT

  std::array<char, 3> byte_std_arr{'a', 'b', 'c'};
  const std::array<char, 3> const_byte_std_arr{'a', 'b', 'c'};
  std::array<int, 3> non_byte_std_arr{1, 2, 3};
  const std::array<int, 3> const_non_byte_std_arr{1, 2, 3};

  SECTION("Iterator Constructor CTAD") {
    // Iterator + count (byte-like)
    auto bs1 = byte_span{byte_vec.begin(), byte_vec.size()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    auto bs2 = byte_span{const_byte_vec.begin(), const_byte_vec.size()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // Iterator + count (non-byte)
    auto bs3 = byte_span{non_byte_vec.begin(), non_byte_vec.size()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<std::byte>>);

    auto bs4 = byte_span{const_non_byte_vec.begin(), const_non_byte_vec.size()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);

    // Iterator + sentinel (byte-like)
    auto bs5 = byte_span{byte_vec.begin(), byte_vec.end()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs5), byte_span<std::byte>>);

    auto bs6 = byte_span{const_byte_vec.begin(), const_byte_vec.end()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs6), byte_span<const std::byte>>);

    // Iterator + sentinel (non-byte)
    auto bs7 = byte_span{non_byte_vec.begin(), non_byte_vec.end()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs7), byte_span<std::byte>>);

    auto bs8 = byte_span{const_non_byte_vec.begin(), const_non_byte_vec.end()};
    STATIC_REQUIRE(std::is_same_v<decltype(bs8), byte_span<const std::byte>>);
  }

  SECTION("C-style Array CTAD") {
    // Byte-like arrays
    auto bs1 = byte_span{byte_arr};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte, 3>>);

    auto bs2 = byte_span{const_byte_arr};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs2), byte_span<const std::byte, 3>>);

    // Non-byte arrays
    auto bs3 = byte_span{non_byte_arr};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs3), byte_span<std::byte, sizeof(int) * 3>>);

    auto bs4 = byte_span{const_non_byte_arr};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4),
                                  byte_span<const std::byte, sizeof(int) * 3>>);
  }

  SECTION("std::array CTAD") {
    // Byte-like std::array
    auto bs1 = byte_span{byte_std_arr};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte, 3>>);

    auto bs2 = byte_span{const_byte_std_arr};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs2), byte_span<const std::byte, 3>>);

    // Non-byte std::array
    auto bs3 = byte_span{non_byte_std_arr};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs3), byte_span<std::byte, sizeof(int) * 3>>);

    auto bs4 = byte_span{const_non_byte_std_arr};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4),
                                  byte_span<const std::byte, sizeof(int) * 3>>);
  }

  SECTION("std::span CTAD") {
    // Byte-like spans
    auto s1 = std::span{byte_vec};
    auto bs1 = byte_span{s1};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    auto s2 = std::span{const_byte_vec};
    auto bs2 = byte_span{s2};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // Non-byte spans
    auto s3 = std::span{non_byte_vec};
    auto bs3 = byte_span{s3};
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<std::byte>>);

    auto s4 = std::span{const_non_byte_vec};
    auto bs4 = byte_span{s4};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);

    // Static extent spans
    auto s5 = std::span<char, 3>{byte_std_arr};
    auto bs5 = byte_span{s5};
    STATIC_REQUIRE(std::is_same_v<decltype(bs5), byte_span<std::byte, 3>>);

    auto s6 = std::span<const int, 3>{const_non_byte_std_arr};
    auto bs6 = byte_span{s6};
    STATIC_REQUIRE(std::is_same_v<decltype(bs6),
                                  byte_span<const std::byte, sizeof(int) * 3>>);
  }

  SECTION("void* CTAD") {
    void* data = byte_vec.data();
    const void* const_data = const_byte_vec.data();

    auto bs1 = byte_span{data, size_t{3}};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    auto bs2 = byte_span{const_data, size_t{3}};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);
  }

  SECTION("Range Constructor CTAD") {
    // Byte-like range
    auto bs1 = byte_span{byte_vec};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    auto bs2 = byte_span{const_byte_vec};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // Non-byte range
    auto bs3 = byte_span{non_byte_vec};
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<std::byte>>);

    auto bs4 = byte_span{const_non_byte_vec};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);
  }

  SECTION("std::span const variations CTAD") {
    std::vector<std::byte> vec(3);

    // Non-const span, non-const element
    std::span<std::byte> s1{vec};
    auto bs1 = byte_span{s1};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    // Non-const span, const element
    std::span<const std::byte> s2{vec};
    auto bs2 = byte_span{s2};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // Const span, non-const element
    const std::span<std::byte> s3{vec};
    auto bs3 = byte_span{s3};
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<std::byte>>);

    // Const span, const element
    const std::span<const std::byte> s4{vec};
    auto bs4 = byte_span{s4};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);

    // Static extent variations
    std::array<std::byte, 3> arr = {std::byte{1}, std::byte{2}, std::byte{3}};

    // Non-const span, non-const element, static extent
    std::span<std::byte, 3> s5{arr};
    auto bs5 = byte_span{s5};
    STATIC_REQUIRE(std::is_same_v<decltype(bs5), byte_span<std::byte, 3>>);

    // Const span, const element, static extent
    const std::span<const std::byte, 3> s6{arr};
    auto bs6 = byte_span{s6};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs6), byte_span<const std::byte, 3>>);
  }

  SECTION("byte_span const variations CTAD") {
    std::vector<std::byte> vec(3);

    // Non-const span, non-const element
    byte_span<std::byte> s1{vec};
    auto bs1 = byte_span{s1};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<std::byte>>);

    // Non-const span, const element
    byte_span<const std::byte> s2{vec};
    auto bs2 = byte_span{s2};
    STATIC_REQUIRE(std::is_same_v<decltype(bs2), byte_span<const std::byte>>);

    // Const span, non-const element
    const byte_span<std::byte> s3{vec};
    auto bs3 = byte_span{s3};
    STATIC_REQUIRE(std::is_same_v<decltype(bs3), byte_span<std::byte>>);

    // Const span, const element
    const byte_span<const std::byte> s4{vec};
    auto bs4 = byte_span{s4};
    STATIC_REQUIRE(std::is_same_v<decltype(bs4), byte_span<const std::byte>>);

    // Static extent variations
    std::array<std::byte, 3> arr = {std::byte{1}, std::byte{2}, std::byte{3}};

    // Non-const span, non-const element, static extent
    byte_span<std::byte, 3> s5{arr};
    auto bs5 = byte_span{s5};
    STATIC_REQUIRE(std::is_same_v<decltype(bs5), byte_span<std::byte, 3>>);

    // Non-const span, const element, static extent
    byte_span<const std::byte, 3> s6{arr};
    auto bs6 = byte_span{s6};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs6), byte_span<const std::byte, 3>>);

    // Const span, non-const element, static extent
    const byte_span<std::byte, 3> s7{arr};
    auto bs7 = byte_span{s7};
    STATIC_REQUIRE(std::is_same_v<decltype(bs7), byte_span<std::byte, 3>>);

    // Const span, const element, static extent
    const byte_span<const std::byte, 3> s8{arr};
    auto bs8 = byte_span{s8};
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs8), byte_span<const std::byte, 3>>);

    // Dynamic to static extent conversion
    byte_span<std::byte> s9{vec};
    auto bs9 = byte_span<std::byte, 3>{s9};  // explicit conversion
    STATIC_REQUIRE(std::is_same_v<decltype(bs9), byte_span<std::byte, 3>>);

    const byte_span<const std::byte> s10{vec};
    auto bs10 = byte_span<const std::byte, 3>{s10};  // explicit conversion
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs10), byte_span<const std::byte, 3>>);
  }

  SECTION("std::string_view CTAD") {
    using namespace std::string_view_literals;

    // Basic string_view
    auto sv = std::string_view{"Hello"};
    auto bs1 = byte_span{sv};
    STATIC_REQUIRE(std::is_same_v<decltype(bs1), byte_span<const std::byte>>);

    // string_view literals
    auto bs_lit1 = byte_span{"Hello"sv};
    auto bs_lit2 = byte_span{L"Hello"sv};
    auto bs_lit3 = byte_span{u8"Hello"sv};
    auto bs_lit4 = byte_span{u"Hello"sv};
    auto bs_lit5 = byte_span{U"Hello"sv};

    STATIC_REQUIRE(
        std::is_same_v<decltype(bs_lit1), byte_span<const std::byte>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs_lit2), byte_span<const std::byte>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs_lit3), byte_span<const std::byte>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs_lit4), byte_span<const std::byte>>);
    STATIC_REQUIRE(
        std::is_same_v<decltype(bs_lit5), byte_span<const std::byte>>);

    // const string_view
    const auto csv = std::string_view{"World"};
    auto bs6 = byte_span{csv};
    STATIC_REQUIRE(std::is_same_v<decltype(bs6), byte_span<const std::byte>>);
  }
}

// NOLINTEND(misc-const-correctness)
