cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../niiet_cmake/cmake/niiet_gcc.cmake)

project(uart_echo CXX C ASM)

niiet_fetch_cmsis(K1921)
find_package(CMSIS COMPONENTS K1921VG015 REQUIRED)

add_executable(uart_echo src/main.cpp)
target_compile_features(uart_echo PRIVATE cxx_std_20)

# Board (IRIS UNO-VG015) has a 12 MHz crystal.
set(BOARD_HSE 12000000)
target_compile_definitions(uart_echo PRIVATE HSE_VALUE=${BOARD_HSE})
target_compile_definitions(cmsis_k1921vg015 PRIVATE
    HSECLK_VAL=12000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
target_include_directories(uart_echo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(uart_echo PRIVATE
    CMSIS::K1921VG015
    NIIET::Nano
    NIIET::NoSys)

niiet_k1921vg015_target_link_flash(uart_echo)
niiet_print_size_of_target(uart_echo)
niiet_generate_hex_file(uart_echo)
