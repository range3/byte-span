cmake_minimum_required(VERSION 3.14)

project(byte-spanTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(byte-span REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/source/*_test.cpp")

add_executable(byte-span_test ${TEST_SOURCES})
target_link_libraries(
    byte-span_test PRIVATE
    byte-span::byte-span
    Catch2::Catch2WithMain
)
target_compile_features(byte-span_test PRIVATE cxx_std_20)

catch_discover_tests(byte-span_test)

# ---- End-of-file commands ----

add_folders(Test)
