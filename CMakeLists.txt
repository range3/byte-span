cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    lsm-tree
    VERSION 0.1.0
    DESCRIPTION "LSM-tree"
    HOMEPAGE_URL "https://github.com/range3/lsm-tree"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(lsm-tree_lsm-tree INTERFACE)
add_library(lsm-tree::lsm-tree ALIAS lsm-tree_lsm-tree)

set_property(
    TARGET lsm-tree_lsm-tree PROPERTY
    EXPORT_NAME lsm-tree
)

target_include_directories(
    lsm-tree_lsm-tree ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(lsm-tree_lsm-tree INTERFACE cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(lsm-tree_lsm-tree INTERFACE fmt::fmt)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT lsm-tree_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of lsm-tree"
  )
endif()

include(cmake/dev-mode.cmake)
