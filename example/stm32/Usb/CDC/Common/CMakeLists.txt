cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../stm32-cmake/cmake/stm32_gcc.cmake)
set(CMAKE_CXX_STANDARD 23)

project(usb_cdc_common CXX C ASM)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include)

set(_usb_compile_opts -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)

add_executable(usb_cdc_f0 CdcExample.cpp)
target_link_libraries(usb_cdc_f0 CMSIS::STM32::F072RB STM32::NoSys STM32::Nano)
target_compile_options(usb_cdc_f0 PRIVATE ${_usb_compile_opts})
stm32_print_size_of_target(usb_cdc_f0)

add_executable(usb_cdc_f1 CdcExample_f1.cpp)
target_link_libraries(usb_cdc_f1 CMSIS::STM32::F103C8 STM32::NoSys STM32::Nano)
target_compile_options(usb_cdc_f1 PRIVATE ${_usb_compile_opts})
stm32_print_size_of_target(usb_cdc_f1)

add_executable(usb_cdc_f4 CdcExample_f4.cpp)
target_link_libraries(usb_cdc_f4 CMSIS::STM32::F401CC STM32::NoSys STM32::Nano)
target_compile_options(usb_cdc_f4 PRIVATE ${_usb_compile_opts})
stm32_print_size_of_target(usb_cdc_f4)

# H5 build (STM32H503CB)
add_executable(usb_cdc_h503 CdcExample_h503.cpp)
target_link_libraries(usb_cdc_h503 CMSIS::STM32::H503CB STM32::NoSys STM32::Nano)
target_compile_definitions(usb_cdc_h503 PRIVATE F_CPU=32000000)
target_compile_options(usb_cdc_h503 PRIVATE ${_usb_compile_opts})
stm32_print_size_of_target(usb_cdc_h503)
