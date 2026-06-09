cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../niiet_cmake/cmake/niiet_gcc.cmake)

project(pinlist CXX C ASM)

niiet_fetch_cmsis(K1921)
find_package(CMSIS COMPONENTS K1921VG015 REQUIRED)

add_executable(pinlist src/main.cpp)
target_compile_features(pinlist PRIVATE cxx_std_20)

# --- Board / application configuration (not part of the silicon) ---
# This board (IRIS/UNO-VG015) has a 12 MHz crystal.
set(BOARD_HSE 12000000)
target_compile_definitions(pinlist PRIVATE HSE_VALUE=${BOARD_HSE})  # Zhele headers
target_compile_definitions(cmsis_k1921vg015 PRIVATE                # NIIET SDK sources
    HSECLK_VAL=${BOARD_HSE}   # crystal, used by system_k1921vg015.c
    SYSCLK_PLL                # derive SYSCLK from PLL0
    CKO_NONE                  # no clock-out pin
    USE_LIBC)
target_include_directories(pinlist PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(pinlist PRIVATE
    CMSIS::K1921VG015
    NIIET::Nano
    NIIET::NoSys)

niiet_k1921vg015_target_link_flash(pinlist)
niiet_print_size_of_target(pinlist)
niiet_generate_hex_file(pinlist)
