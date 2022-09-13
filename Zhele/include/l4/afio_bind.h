/**
 * @file
 * Implements utils for STM32L4 afio.
 * GPIO alternate functions have hard bind.
 * So, there is easy to move afio resolve in compile-time.
 * 
 * @author Alexey Zhelonkin (based on Konstantin Chizhov)
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_AFIO_BIND_H
#define ZHELE_AFIO_BIND_H

#include "../common/template_utils/static_array.h"
#include "../common/template_utils/type_list.h"

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

    using Regs = Zhele::TemplateUtils::TypeList<
        Usart1Regs, Usart2Regs, Usart3Regs, Uart4Regs, Uart5Regs, Usart6Regs, // Usart
        Spi1Regs, Spi2Regs, Spi3Regs, // SPI
        I2C1Regs, I2C2Regs, I2C3Regs, // I2C
        UsbRegs // USB_FS
    >;
    using AltFunctionNumbers = Zhele::TemplateUtils::NonTypeTemplateArray<
        7, 7, 7, 8, 8, 8, // Usart
        5, 5, 6, // SPI
        4, 4, 4, // I2C
        10 // USB_FS
    >;

    template <typename _Regs>
    struct AltFuncHelper
    {
        const static uint8_t value = static_cast<uint8_t>(GetNonTypeValueByIndex<TemplateUtils::TypeIndex<_Regs, Regs>::value, AltFunctionNumbers>::value);
    };

    template<typename Regs>
    const static uint8_t GetAltFunctionNumber = AltFuncHelper<Regs>::value;
} // namespace Private


#endif // !ZHELE_AFIO_BIND_H