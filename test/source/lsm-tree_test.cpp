#include <catch2/catch_test_macros.hpp>

#include "lsm-tree/lsm-tree.hpp"

TEST_CASE("Name is lsm-tree", "[library]") {
  REQUIRE(name() == "lsm-tree");
}
