#include "lsm-tree/lsm-tree.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is lsm-tree", "[library]")
{
  REQUIRE(name() == "lsm-tree");
}
