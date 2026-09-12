cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../niiet_cmake/cmake/niiet_gcc.cmake)

project(uart_echo_dma CXX C ASM)

niiet_fetch_cmsis(K1921)
niiet_fetch_cmsis(K1921VG5T)
niiet_fetch_cmsis(K1921VG7T)
find_package(CMSIS COMPONENTS K1921VG015 K1921VG5T K1921VG7T REQUIRED)

# K1921VG015 on IRIS UNO-VG015 (12 MHz crystal)
target_compile_definitions(cmsis_k1921vg015 PRIVATE
    HSECLK_VAL=12000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(uart_echo_dma_vg015 src/main.cpp)
target_compile_features(uart_echo_dma_vg015 PRIVATE cxx_std_20)
target_compile_definitions(uart_echo_dma_vg015 PRIVATE HSE_VALUE=12000000)
target_include_directories(uart_echo_dma_vg015 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(uart_echo_dma_vg015 PRIVATE
    CMSIS::K1921VG015
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg015_target_link_flash(uart_echo_dma_vg015)
niiet_print_size_of_target(uart_echo_dma_vg015)
niiet_generate_hex_file(uart_echo_dma_vg015)

# K1921VG5T on NIIET-MINI-K1921VG5T (16 MHz crystal)
target_compile_definitions(cmsis_k1921vg5t PRIVATE
    HSECLK_VAL=16000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(uart_echo_dma_vg5t src/main.cpp)
target_compile_features(uart_echo_dma_vg5t PRIVATE cxx_std_20)
target_compile_definitions(uart_echo_dma_vg5t PRIVATE HSE_VALUE=16000000)
target_include_directories(uart_echo_dma_vg5t PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(uart_echo_dma_vg5t PRIVATE
    CMSIS::K1921VG5T
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg5t_target_link_flash(uart_echo_dma_vg5t)
niiet_print_size_of_target(uart_echo_dma_vg5t)
niiet_generate_hex_file(uart_echo_dma_vg5t)

# K1921VG7T on NIIET-MINI-K1921VG7T (16 MHz crystal)
target_compile_definitions(cmsis_k1921vg7t PRIVATE
    HSECLK_VAL=16000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(uart_echo_dma_vg7t src/main.cpp)
target_compile_features(uart_echo_dma_vg7t PRIVATE cxx_std_20)
target_compile_definitions(uart_echo_dma_vg7t PRIVATE HSE_VALUE=16000000)
target_include_directories(uart_echo_dma_vg7t PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(uart_echo_dma_vg7t PRIVATE
    CMSIS::K1921VG7T
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg7t_target_link_flash(uart_echo_dma_vg7t)
niiet_print_size_of_target(uart_echo_dma_vg7t)
niiet_generate_hex_file(uart_echo_dma_vg7t)
