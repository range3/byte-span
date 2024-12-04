#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/byte_view.hpp"

using lsm::utils::byte_view;
using lsm::utils::cbyte_view;

TEST_CASE("byte_view basic functionality", "[byte_view]") {
  std::array<std::byte, 3> data{std::byte{1}, std::byte{2}, std::byte{3}};

  SECTION("construct and basic accessors") {
    auto const view = byte_view(data.data(), data.size());

    REQUIRE(view.data() == data.data());
    REQUIRE(view.size() == 3);
    REQUIRE_FALSE(view.empty());
  }

  SECTION("empty view") {
    auto const view = byte_view(nullptr, 0);

    REQUIRE(view.data() == nullptr);
    REQUIRE(view.size() == 0);  // NOLINT
    REQUIRE(view.empty());
  }

  SECTION("element access with operator[]") {
    auto const view = byte_view(data.data(), data.size());

    REQUIRE(std::to_integer<int>(view[0]) == 1);
    REQUIRE(std::to_integer<int>(view[1]) == 2);
    REQUIRE(std::to_integer<int>(view[2]) == 3);

    // writable
    view[1] = std::byte{42};
    REQUIRE(std::to_integer<int>(view[1]) == 42);
    REQUIRE(std::to_integer<int>(data[1]) == 42);
  }

  SECTION("element access with at()") {
    auto const view = byte_view(data.data(), data.size());

    REQUIRE(std::to_integer<int>(view.at(0)) == 1);
    REQUIRE(std::to_integer<int>(view.at(1)) == 2);
    REQUIRE(std::to_integer<int>(view.at(2)) == 3);

    REQUIRE_THROWS_AS(view.at(3), std::out_of_range);

    // writable
    view.at(1) = std::byte{42};
    REQUIRE(std::to_integer<int>(view.at(1)) == 42);
    REQUIRE(std::to_integer<int>(data[1]) == 42);
  }

  SECTION("front/back access") {
    auto const view = byte_view(data.data(), data.size());

    REQUIRE(std::to_integer<int>(view.front()) == 1);
    REQUIRE(std::to_integer<int>(view.back()) == 3);

    // writable
    view.front() = std::byte{42};
    view.back() = std::byte{43};
    REQUIRE(std::to_integer<int>(data[0]) == 42);
    REQUIRE(std::to_integer<int>(data[2]) == 43);
  }

  SECTION("iterator operations") {
    auto const view = byte_view(data.data(), data.size());

    // iterator
    REQUIRE(std::distance(view.begin(), view.end()) == 3);
    REQUIRE(std::to_integer<int>(*view.begin()) == 1);
    REQUIRE(std::to_integer<int>(*(view.end() - 1)) == 3);

    // writable
    *view.begin() = std::byte{42};
    REQUIRE(std::to_integer<int>(data[0]) == 42);

    // const_iterator
    REQUIRE(std::distance(view.cbegin(), view.cend()) == 3);
    REQUIRE(std::to_integer<int>(*view.cbegin()) == 42);

    // reverse_iterator
    auto rbegin = view.rbegin();
    auto rend = view.rend();
    REQUIRE(std::distance(rbegin, rend) == 3);
    REQUIRE(std::to_integer<int>(*rbegin) == 3);
    REQUIRE(std::to_integer<int>(*(rend - 1)) == 42);

    // const_reverse_iterator
    auto crbegin = view.crbegin();
    auto crend = view.crend();
    REQUIRE(std::distance(crbegin, crend) == 3);
    REQUIRE(std::to_integer<int>(*crbegin) == 3);

    // compatible with std::algorithms
    std::array<std::byte, 3> expected{std::byte{42}, std::byte{2},
                                      std::byte{3}};
    REQUIRE(std::equal(view.begin(), view.end(), expected.begin()));

    // reverse compatible with std::algorithms
    std::array<std::byte, 3> reversed{std::byte{3}, std::byte{2},
                                      std::byte{42}};
    REQUIRE(std::equal(view.rbegin(), view.rend(), reversed.begin()));
  }
}
