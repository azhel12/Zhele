cmake_minimum_required(VERSION 3.16)

set (CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../stm32-cmake/cmake/stm32_gcc.cmake)
set (CMAKE_CXX_STANDARD 23)

project(usart_common CXX C ASM)

# Populate CMSIS using stm32-cmake project (Commented for use in github actions, uncomment if you want to build example alone)
#stm32_fetch_cmsis(C0 F0 F1 F4 G0)
#find_package(CMSIS COMPONENTS STM32C011F6 STM32F0 STM32F1 STM32F4 STM32G0 REQUIRED)

# Add zhele as include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../include)

# C0 build (STM32C011, USART2 on PA2/PA3, HSI48 @ 48 MHz)
add_executable(usart_common_c011 main.cpp)
target_link_libraries(usart_common_c011 CMSIS::STM32::C011F6 STM32::NoSys STM32::Nano)
target_compile_definitions(usart_common_c011 PRIVATE F_CPU=48000000)
target_compile_options(usart_common_c011 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(usart_common_c011)
stm32_generate_hex_file(usart_common_c011)

# F0 build
add_executable(usart_common_f0 main.cpp)
target_link_libraries(usart_common_f0 CMSIS::STM32::F072RB STM32::NoSys STM32::Nano)
target_compile_definitions(usart_common_f0 PRIVATE HSI_VALUE=8000000)
target_compile_options(usart_common_f0 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(usart_common_f0)

# F1 build
add_executable(usart_common_f1 main.cpp)
target_link_libraries(usart_common_f1 CMSIS::STM32::F103C8 STM32::NoSys STM32::Nano)
target_compile_definitions(usart_common_f1 PRIVATE HSI_VALUE=8000000)
target_compile_options(usart_common_f1 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(usart_common_f1)

# F4 build
add_executable(usart_common_f4 main.cpp)
target_link_libraries(usart_common_f4 CMSIS::STM32::F401CC STM32::NoSys STM32::Nano)
target_compile_definitions(usart_common_f4 PRIVATE HSI_VALUE=16000000)
target_compile_options(usart_common_f4 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(usart_common_f4)

# G0 build (TODO:: fix USART for G0)
add_executable(usart_common_g0 main.cpp)
target_link_libraries(usart_common_g0 CMSIS::STM32::G030F6 STM32::NoSys STM32::Nano)
target_compile_definitions(usart_common_g0 PRIVATE HSI_VALUE=16000000)
target_compile_options(usart_common_g0 PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
stm32_print_size_of_target(usart_common_g0)