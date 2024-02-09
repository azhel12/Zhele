cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../stm32-cmake/cmake/stm32_gcc.cmake)
set (CMAKE_CXX_STANDARD 23)

project(adc_regular_cdc CXX C ASM)

# Populate CMSIS using stm32-cmake project (Commented for use in github actions, uncomment if you want to build example alone)
#stm32_fetch_cmsis(F0 F1 F4)
#find_package(CMSIS COMPONENTS STM32F0 STM32F1 STM32F4 REQUIRED)


# Add zhele as include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

# F0 build (ADC not supported yet)
#add_executable(adc_regular_cdc_f0 main.cpp)
#target_link_libraries(adc_regular_cdc_f0 CMSIS::STM32::F072RB STM32::NoSys STM32::Nano)
#target_compile_definitions(adc_regular_cdc_f0 PRIVATE F_CPU=8000000) # Need for delay
#target_compile_options(adc_regular_cdc_f0 PRIVATE -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)
#stm32_print_size_of_target(adc_regular_cdc_f0)

# F1 build
add_executable(adc_regular_cdc_f1 main.cpp)
target_link_libraries(adc_regular_cdc_f1 CMSIS::STM32::F103C8 STM32::NoSys STM32::Nano)
target_compile_definitions(adc_regular_cdc_f1 PRIVATE F_CPU=8000000) # Need for delay
target_compile_options(adc_regular_cdc_f1 PRIVATE -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)
stm32_print_size_of_target(adc_regular_cdc_f1)

# F4 build (ADC not supported yet)
#add_executable(adc_regular_cdc_f4 main.cpp)
#target_link_libraries(adc_regular_cdc_f4 CMSIS::STM32::F401CC STM32::NoSys STM32::Nano)
#target_compile_definitions(adc_regular_cdc_f4 PRIVATE F_CPU=16000000) # Need for delay
#target_compile_options(adc_regular_cdc_f4 PRIVATE -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)
#stm32_print_size_of_target(adc_regular_cdc_f4)