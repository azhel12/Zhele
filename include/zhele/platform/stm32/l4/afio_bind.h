/**
 * @file
 * Implements utils for STM32L4 afio
 * GPIO alternate functions have hard bind
 * So, there is easy to move afio resolve in compile-time
 * 
 * @author Alexey Zhelonkin (based on Konstantin Chizhov)
 * @date 2022
 * @license MIT
 */

#pragma once

#include <zhele/common/template_utils/type_list.h>

#include <cstdint>

namespace Zhele::Private
{
    // USART
    class Usart1Regs; class Usart2Regs; class Usart3Regs; class Uart4Regs; class Uart5Regs; class Usart6Regs;
    // SPI
    class Spi1Regs; class Spi2Regs; class Spi3Regs;
    // I2C
    class I2C1Regs; class I2C2Regs; class I2C3Regs; 
    // USB
    class UsbRegs;

    using Regs = Zhele::template_utils::type_list<
        Usart1Regs, Usart2Regs, Usart3Regs, Uart4Regs, Uart5Regs, Usart6Regs, // Usart
        Spi1Regs, Spi2Regs, Spi3Regs, // SPI
        I2C1Regs, I2C2Regs, I2C3Regs, // I2C
        UsbRegs // USB_FS
    >;

    inline constexpr std::array<uint8_t, 13> alt_functions{
        7, 7, 7, 8, 8, 8, // Usart
        5, 5, 6, // SPI
        4, 4, 4, // I2C
        10 // USB_FS
    };

    template <typename _Regs>
    struct AltFuncHelper
    {
        static constexpr uint8_t value = alt_functions[Regs::template search<_Regs>()];
    };

    template<typename Regs>
    const static uint8_t GetAltFunctionNumber = AltFuncHelper<Regs>::value;
} // namespace Private
