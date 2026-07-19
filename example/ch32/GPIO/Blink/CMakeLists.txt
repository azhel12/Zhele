cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../wch_cmake/cmake/wch_gcc.cmake)

project(blink CXX C ASM)

ch32_fetch_cmsis(V0 V2)
find_package(CMSIS COMPONENTS CH32V0 CH32V2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)

add_executable(blink_v003 main.cpp)
target_compile_features(blink_v003 PRIVATE cxx_std_23)
target_link_libraries(blink_v003 CMSIS::CH32::V003F4 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(blink_v003)
ch32_generate_hex_file(blink_v003)

add_executable(blink_v006 main.cpp)
target_compile_features(blink_v006 PRIVATE cxx_std_23)
target_link_libraries(blink_v006 CMSIS::CH32::V006K8 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(blink_v006)
ch32_generate_hex_file(blink_v006)

add_executable(blink_v203 main.cpp)
target_compile_features(blink_v203 PRIVATE cxx_std_23)
target_compile_definitions(blink_v203 PRIVATE CH32V203)
target_link_libraries(blink_v203 CMSIS::CH32::V203C8 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(blink_v203)
ch32_generate_hex_file(blink_v203)
