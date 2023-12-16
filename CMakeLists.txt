cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    zhele
    VERSION 0.1.0
    DESCRIPTION "Header-only stm32 framework based on modern c++."
    HOMEPAGE_URL "https://github.com/azhel12/Zhele/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(zhele_zhele INTERFACE)
add_library(zhele::zhele ALIAS zhele_zhele)

set_property(
    TARGET zhele_zhele PROPERTY
    EXPORT_NAME zhele
)

target_include_directories(
    zhele_zhele ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(zhele_zhele INTERFACE cxx_std_23)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${zhele_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT zhele_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of zhele"
  )
endif()

include(cmake/dev-mode.cmake)
