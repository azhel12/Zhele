cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../niiet_cmake/cmake/niiet_gcc.cmake)

project(pinlist CXX C ASM)

niiet_fetch_cmsis(K1921)
find_package(CMSIS COMPONENTS K1921VG015 REQUIRED)

# K1921VG015 on IRIS UNO-VG015 (12 MHz crystal)
target_compile_definitions(cmsis_k1921vg015 PRIVATE
    HSECLK_VAL=12000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(pinlist_vg015 src/main.cpp)
target_compile_features(pinlist_vg015 PRIVATE cxx_std_20)
target_compile_definitions(pinlist_vg015 PRIVATE HSE_VALUE=12000000)
target_include_directories(pinlist_vg015 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(pinlist_vg015 PRIVATE
    CMSIS::K1921VG015
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg015_target_link_flash(pinlist_vg015)
niiet_print_size_of_target(pinlist_vg015)
niiet_generate_hex_file(pinlist_vg015)
