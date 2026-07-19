cmake_minimum_required(VERSION 3.16)

# Default to the WCH toolchain — the TIM2 ISR uses "WCH-Interrupt-fast". Override
# with -DCH32_TARGET_TRIPLET=riscv-none-elf to fall back to naked + mret.
set(CH32_TARGET_TRIPLET riscv32-wch-elf CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../wch_cmake/cmake/wch_gcc.cmake)

project(clock_blink CXX C ASM)

# Only the WCH GCC understands interrupt("WCH-Interrupt-fast"); tell the source so
# it can fall back to naked + mret on the generic toolchain.
if(CMAKE_C_COMPILER MATCHES "riscv32-wch-elf")
    add_compile_definitions(ZHELE_WCH_TOOLCHAIN)
endif()

ch32_fetch_cmsis(V0 V2)
find_package(CMSIS COMPONENTS CH32V0 CH32V2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)

add_executable(clock_blink_v003 main.cpp)
target_compile_features(clock_blink_v003 PRIVATE cxx_std_23)
target_link_libraries(clock_blink_v003 CMSIS::CH32::V003F4 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(clock_blink_v003)
ch32_generate_hex_file(clock_blink_v003)

add_executable(clock_blink_v006 main.cpp)
target_compile_features(clock_blink_v006 PRIVATE cxx_std_23)
target_link_libraries(clock_blink_v006 CMSIS::CH32::V006K8 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(clock_blink_v006)
ch32_generate_hex_file(clock_blink_v006)

# ch32v20x.h self-defines CH32V20x_D6, but Zhele's platform detector runs before
# the device header is pulled in, so name the family on the command line.
add_executable(clock_blink_v203 main.cpp)
target_compile_features(clock_blink_v203 PRIVATE cxx_std_23)
target_compile_definitions(clock_blink_v203 PRIVATE CH32V203)
target_link_libraries(clock_blink_v203 CMSIS::CH32::V203C8 CH32::Nano CH32::NoSys)
ch32_print_size_of_target(clock_blink_v203)
ch32_generate_hex_file(clock_blink_v203)
