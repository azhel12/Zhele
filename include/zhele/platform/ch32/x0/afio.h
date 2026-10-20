/**
 * @file
 * @brief Implements AFIO remap for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_AFIO_H
#define ZHELE_PLATFORM_CH32_X0_AFIO_H

#include <ch32x035.h>

#include "clock.h"

#include "../common/ioreg.h"

namespace Zhele::IO::Private
{
    template<typename Clock> struct PeriphRemapField;

    #define ZHELE_CH32_X0_REMAP(CLOCK, MASK) \
        IO_BITFIELD_WRAPPER(AFIO->PCFR1, CLOCK##Remap, uint32_t, \
            __builtin_ctz(static_cast<uint32_t>(MASK)), \
            GetBitFieldLength<(static_cast<uint32_t>(MASK) >> __builtin_ctz(static_cast<uint32_t>(MASK)))>); \
        template<> struct PeriphRemapField<Zhele::Clock::CLOCK> { using type = CLOCK##Remap; }

    ZHELE_CH32_X0_REMAP(Spi1Clock, AFIO_PCFR1_SPI1_REMAP);
    ZHELE_CH32_X0_REMAP(I2c1Clock, AFIO_PCFR1_I2C1_REMAP);
    ZHELE_CH32_X0_REMAP(Usart1Clock, AFIO_PCFR1_USART1_REMAP);
    ZHELE_CH32_X0_REMAP(Usart2Clock, AFIO_PCFR1_USART2_REMAP);
    ZHELE_CH32_X0_REMAP(Usart3Clock, AFIO_PCFR1_USART3_REMAP);
    ZHELE_CH32_X0_REMAP(Usart4Clock, AFIO_PCFR1_USART4_REMAP);
    ZHELE_CH32_X0_REMAP(Tim1Clock, AFIO_PCFR1_TIM1_REMAP);
    ZHELE_CH32_X0_REMAP(Tim2Clock, AFIO_PCFR1_TIM2_REMAP);
    ZHELE_CH32_X0_REMAP(Tim3Clock, AFIO_PCFR1_TIM3_REMAP);

    #undef ZHELE_CH32_X0_REMAP

    template<typename Clock>
    using PeriphRemap = typename PeriphRemapField<Clock>::type;
}

#endif // ZHELE_PLATFORM_CH32_X0_AFIO_H
