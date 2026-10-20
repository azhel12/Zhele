/**
 * @file
 * @brief Implements GPIO ports for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_IOPORTS_H
#define ZHELE_PLATFORM_CH32_X0_IOPORTS_H

#include <ch32x035.h>

#include "clock.h"

#include "../common/ioreg.h"
#include "../common/ioports_gpio.h"

namespace Zhele::IO
{
#define MAKE_PORT(REGS, ClkEnReg, className, ID) \
    namespace Private \
    { \
        IO_STRUCT_WRAPPER(REGS, className##Regs, GPIO_TypeDef); \
    } \
    using className = Private::PortImplementation<Private::className##Regs, ClkEnReg, ID>;

    MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
    MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
    MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')

#undef MAKE_PORT
}

#endif // ZHELE_PLATFORM_CH32_X0_IOPORTS_H
