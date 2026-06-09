cmake_minimum_required(VERSION 3.16)

set (CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../stm32-cmake/cmake/stm32_gcc.cmake)
set (CMAKE_CXX_STANDARD 23)

project(led_control CXX C ASM)

# Populate CMSIS using stm32-cmake project (Commented for use in github actions, uncomment if you want to build example alone)
#stm32_fetch_cmsis(C0 F0 F1 F4 G0)
#find_package(CMSIS COMPONENTS STM32C011F6 STM32F0 STM32F1 STM32F4 STM32G0 REQUIRED)

# Add zhele as include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)

# C0 build (STM32C011, HSI48 @ 48 MHz; LED on PA4 wired active-low)
add_executable(led_control_c011 main.cpp)
target_link_libraries(led_control_c011 CMSIS::STM32::C011F6 STM32::NoSys STM32::Nano)
target_compile_definitions(led_control_c011 PRIVATE F_CPU=48000000)
target_compile_options(led_control_c011 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(led_control_c011)
stm32_generate_hex_file(led_control_c011)

# F0 build
add_executable(led_control_f0 main.cpp)
target_link_libraries(led_control_f0 CMSIS::STM32::F072RB STM32::NoSys STM32::Nano)
target_compile_options(led_control_f0 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(led_control_f0)

# F1 build
add_executable(led_control_f1 main.cpp)
target_link_libraries(led_control_f1 CMSIS::STM32::F103C8 STM32::NoSys STM32::Nano)
target_compile_options(led_control_f1 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(led_control_f1)

# F4 build
add_executable(led_control_f4 main.cpp)
target_link_libraries(led_control_f4 CMSIS::STM32::F401CC STM32::NoSys STM32::Nano)
target_compile_options(led_control_f4 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(led_control_f4)

# G0 build
add_executable(led_control_g0 main.cpp)
target_link_libraries(led_control_g0 CMSIS::STM32::G030F6 STM32::NoSys STM32::Nano)
target_compile_options(led_control_g0 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(led_control_g0)