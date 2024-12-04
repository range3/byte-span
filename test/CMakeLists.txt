cmake_minimum_required(VERSION 3.14)

project(lsm-treeTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(lsm-tree REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/source/*_test.cpp")

add_executable(lsm-tree_test ${TEST_SOURCES})
target_link_libraries(
    lsm-tree_test PRIVATE
    lsm-tree::lsm-tree
    Catch2::Catch2WithMain
)
target_compile_features(lsm-tree_test PRIVATE cxx_std_20)

catch_discover_tests(lsm-tree_test)

# ---- End-of-file commands ----

add_folders(Test)
