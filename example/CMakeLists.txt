cmake_minimum_required(VERSION 3.16)

set (CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../stm32-cmake/cmake/stm32_gcc.cmake)

project(zhele_example CXX C ASM)

# Populate CMSIS using stm32-cmake project for all nested examples
stm32_fetch_cmsis(F0 F1 F4 G0)
find_package(CMSIS COMPONENTS STM32F0 STM32F1 STM32F4 STM32G0 REQUIRED)

add_subdirectory(Adc)
add_subdirectory(Clock)
add_subdirectory(Crc)
add_subdirectory(Dac)
add_subdirectory(Dma)
add_subdirectory(Drivers)
add_subdirectory(Exti)
add_subdirectory(Gpio)
add_subdirectory(I2c)
add_subdirectory(Spi)
add_subdirectory(Timer)
add_subdirectory(Usart)
# add_subdirectory(Usb) TODO:: Combine examples for F0/F1/F4
add_subdirectory(WatchDog)