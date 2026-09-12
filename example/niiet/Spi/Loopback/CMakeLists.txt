cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../niiet_cmake/cmake/niiet_gcc.cmake)

project(spi_loopback CXX C ASM)

niiet_fetch_cmsis(K1921VG5T)
niiet_fetch_cmsis(K1921VG7T)
find_package(CMSIS COMPONENTS K1921VG5T K1921VG7T REQUIRED)

# K1921VG5T on NIIET-MINI-K1921VG5T (16 MHz crystal)
target_compile_definitions(cmsis_k1921vg5t PRIVATE
    HSECLK_VAL=16000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(spi_loopback_vg5t src/main.cpp)
target_compile_features(spi_loopback_vg5t PRIVATE cxx_std_20)
target_compile_definitions(spi_loopback_vg5t PRIVATE HSE_VALUE=16000000)
target_include_directories(spi_loopback_vg5t PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(spi_loopback_vg5t PRIVATE
    CMSIS::K1921VG5T
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg5t_target_link_flash(spi_loopback_vg5t)
niiet_print_size_of_target(spi_loopback_vg5t)
niiet_generate_hex_file(spi_loopback_vg5t)

# K1921VG7T on NIIET-MINI-K1921VG7T (16 MHz crystal)
target_compile_definitions(cmsis_k1921vg7t PRIVATE
    HSECLK_VAL=16000000
    SYSCLK_PLL
    CKO_NONE
    USE_LIBC)
add_executable(spi_loopback_vg7t src/main.cpp)
target_compile_features(spi_loopback_vg7t PRIVATE cxx_std_20)
target_compile_definitions(spi_loopback_vg7t PRIVATE HSE_VALUE=16000000)
target_include_directories(spi_loopback_vg7t PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)
target_link_libraries(spi_loopback_vg7t PRIVATE
    CMSIS::K1921VG7T
    NIIET::Nano
    NIIET::NoSys)
niiet_k1921vg7t_target_link_flash(spi_loopback_vg7t)
niiet_print_size_of_target(spi_loopback_vg7t)
niiet_generate_hex_file(spi_loopback_vg7t)
