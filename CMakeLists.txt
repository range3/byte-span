cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ByteSpan
    VERSION 0.2.0
    DESCRIPTION "Lightweight view class for efficient handling of byte data"
    HOMEPAGE_URL "https://github.com/range3/ByteSpan"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(ByteSpan_ByteSpan INTERFACE)
add_library(ByteSpan::ByteSpan ALIAS ByteSpan_ByteSpan)

set_property(
    TARGET ByteSpan_ByteSpan PROPERTY
    EXPORT_NAME ByteSpan
)

target_include_directories(
    ByteSpan_ByteSpan ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(ByteSpan_ByteSpan INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ByteSpan_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ByteSpan"
  )
endif()

include(cmake/dev-mode.cmake)
