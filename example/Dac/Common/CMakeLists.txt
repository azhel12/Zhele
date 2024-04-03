cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../stm32-cmake/cmake/stm32_gcc.cmake)
set (CMAKE_CXX_STANDARD 23)

project(dac_common CXX C ASM)

# Populate CMSIS using stm32-cmake project (Commented for use in github actions, uncomment if you want to build example alone)
#stm32_fetch_cmsis(F4)
#find_package(CMSIS COMPONENTS STM32F4 REQUIRED)


# Add zhele as include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

# F0 (not support DAC yet)
# F1 (not support DAC yet)
# G0 (not support DAC yet)

# F4 build
add_executable(dac_common_f4 main.cpp)
target_link_libraries(dac_common_f4 CMSIS::STM32::F407VG STM32::NoSys STM32::Nano)
target_compile_options(dac_common_f4 PRIVATE -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)
stm32_print_size_of_target(dac_common_f4)