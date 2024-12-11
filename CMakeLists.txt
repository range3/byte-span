cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    byte-span
    VERSION 0.1.0
    DESCRIPTION "Lightweight view class for efficient handling of byte data"
    HOMEPAGE_URL "https://github.com/range3/byte-span"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(byte-span_byte-span INTERFACE)
add_library(byte-span::byte-span ALIAS byte-span_byte-span)

set_property(
    TARGET byte-span_byte-span PROPERTY
    EXPORT_NAME byte-span
)

target_include_directories(
    byte-span_byte-span ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(byte-span_byte-span INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT byte-span_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of byte-span"
  )
endif()

include(cmake/dev-mode.cmake)
