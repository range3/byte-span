include(FetchContent)
FetchContent_Declare(
    inspector 
    GIT_REPOSITORY https://github.com/range3/inspector.git
    GIT_TAG v0.2.0
    GIT_SHALLOW YES
)

FetchContent_GetProperties(inspector)
if(NOT inspector_POPULATED)
  FetchContent_Populate(inspector)
  list(APPEND CMAKE_MODULE_PATH "${inspector_SOURCE_DIR}/cmake/find")
  add_subdirectory("${inspector_SOURCE_DIR}" "${inspector_BINARY_DIR}")
endif()

set(inspector_FOUND 1)
