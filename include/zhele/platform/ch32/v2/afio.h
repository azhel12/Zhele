/**
 * @file
 * CH32V20x AFIO remap: PeriphRemap<Clock> over AFIO->PCFR1.
 *
 * V20x uses F1-style central remapping — each peripheral has a contiguous field
 * in AFIO->PCFR1. PeriphRemap<PeriphClock> exposes that field (Get/Set the remap
 * value); the peripheral's SelectPins derives the value from the chosen pins.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_AFIO_H
#define ZHELE_PLATFORM_CH32_V2_AFIO_H

#include <ch32v20x.h>

#include "clock.h"
#include "../common/ioreg.h"

namespace Zhele::IO::Private
{
    template<typename Clock> struct PeriphRemapField;

    // Declare an AFIO->PCFR1 bitfield for a peripheral and bind it to its clock
    // type. Offset/length are derived from the CMSIS mask (WCH ships no _Pos/_Msk).
    #define ZHELE_CH32_V2_REMAP(CLOCK, MASK) \
        IO_BITFIELD_WRAPPER(AFIO->PCFR1, CLOCK##Remap, uint32_t, \
            __builtin_ctz(static_cast<uint32_t>(MASK)), \
            GetBitFieldLength<(static_cast<uint32_t>(MASK) >> __builtin_ctz(static_cast<uint32_t>(MASK)))>); \
        template<> struct PeriphRemapField<Zhele::Clock::CLOCK> { using type = CLOCK##Remap; }

    ZHELE_CH32_V2_REMAP(Usart1Clock, AFIO_PCFR1_USART1_REMAP);
    ZHELE_CH32_V2_REMAP(Usart2Clock, AFIO_PCFR1_USART2_REMAP);
#if defined(AFIO_PCFR1_USART3_REMAP)
    ZHELE_CH32_V2_REMAP(Usart3Clock, AFIO_PCFR1_USART3_REMAP);
#endif
    ZHELE_CH32_V2_REMAP(Spi1Clock,   AFIO_PCFR1_SPI1_REMAP);
    ZHELE_CH32_V2_REMAP(I2c1Clock,   AFIO_PCFR1_I2C1_REMAP);
    ZHELE_CH32_V2_REMAP(Tim1Clock,   AFIO_PCFR1_TIM1_REMAP);
    ZHELE_CH32_V2_REMAP(Tim2Clock,   AFIO_PCFR1_TIM2_REMAP);
    ZHELE_CH32_V2_REMAP(Tim3Clock,   AFIO_PCFR1_TIM3_REMAP);
#if defined(AFIO_PCFR1_TIM4_REMAP)
    ZHELE_CH32_V2_REMAP(Tim4Clock,   AFIO_PCFR1_TIM4_REMAP);
#endif
    // SPI2 / I2C2 have no remap field in PCFR1 on V20x.

    #undef ZHELE_CH32_V2_REMAP

    // Peripherals with no PCFR1 remap field (single fixed pin set).
    struct NullRemap
    {
        static void Set(uint32_t) {}
        static uint32_t Get() { return 0; }
    };
    template<> struct PeriphRemapField<Zhele::Clock::Spi2Clock> { using type = NullRemap; };
    template<> struct PeriphRemapField<Zhele::Clock::I2c2Clock> { using type = NullRemap; };

    template<typename Clock>
    using PeriphRemap = typename PeriphRemapField<Clock>::type;
}

#endif // ZHELE_PLATFORM_CH32_V2_AFIO_H
