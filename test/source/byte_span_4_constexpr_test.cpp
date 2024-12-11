// NOLINTBEGIN(misc-const-correctness,clang-analyzer-deadcode.DeadStores,unused-includes)
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202207L  // C++26
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

#include "byte_span/byte_span.hpp"


using range3::as_bytes;
using range3::as_span;
using range3::as_sv;
using range3::as_writable_bytes;
using range3::as_writable_span;
using range3::byte_span;
using range3::byte_view;
using range3::cbyte_view;
using range3::detail::byte_like;
using range3::detail::const_convertible;
using namespace std::string_view_literals;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"



TEST_CASE("byte_span compile-time construction", "[byte_span]") {
    SECTION("from iterator and sentinel") {
        constexpr std::array<char, 4> arr = {'a', 'b', 'c', 'd'};
        constexpr byte_span view(arr.begin(), arr.end());
        STATIC_REQUIRE(view.size() == 4);
        REQUIRE(view.size() == 4);
    }

    SECTION("from iterator and count") {
        constexpr std::array<char, 4> arr = {'a', 'b', 'c', 'd'};
        constexpr byte_span view(arr.begin(), 4);
        STATIC_REQUIRE(view.size() == 4);
        REQUIRE(view.size() == 4);
    }
    
    /*
    SECTION("from std::array") {}
    SECTION("from C array") {}
    SECTION("from std::span") {}
    */
}



#pragma GCC diagnostic pop

#endif
// NOLINTEND(misc-const-correctness,clang-analyzer-deadcode.DeadStores)
