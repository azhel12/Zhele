cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../wch_cmake/cmake/wch_gcc.cmake)

project(pinlist CXX C ASM)

ch32_fetch_cmsis(V0)
find_package(CMSIS COMPONENTS CH32V0 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)

add_executable(pinlist_v003 main.cpp)
target_compile_features(pinlist_v003 PRIVATE cxx_std_20)
target_link_libraries(pinlist_v003 CMSIS::CH32::V003F4 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(pinlist_v003)
ch32_generate_hex_file(pinlist_v003)

add_executable(pinlist_v006 main.cpp)
target_compile_features(pinlist_v006 PRIVATE cxx_std_20)
target_link_libraries(pinlist_v006 CMSIS::CH32::V006K8 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(pinlist_v006)
ch32_generate_hex_file(pinlist_v006)
