/**
 * @file
 * CH32V00x AFIO remap: PeriphRemap<Clock> over AFIO->PCFR1.
 *
 * V00x (V002/004/005/006/007) uses wide contiguous remap fields. V003 uses the
 * F1-style narrow fields, but its USART1 and I2C1 remap values are SPLIT across a
 * low bit and a high bit — handled by SplitRemapField.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_AFIO_H
#define ZHELE_PLATFORM_CH32_V0_AFIO_H

#include <ch32v00x.h>

#include "clock.h"
#include "../common/ioreg.h"

namespace Zhele::IO::Private
{
    template<typename Clock> struct PeriphRemapField;

    // Contiguous AFIO->PCFR1 field (offset/length from the CMSIS mask).
    #define ZHELE_CH32_V0_REMAP(CLOCK, MASK) \
        IO_BITFIELD_WRAPPER(AFIO->PCFR1, CLOCK##Remap, uint32_t, \
            __builtin_ctz(static_cast<uint32_t>(MASK)), \
            GetBitFieldLength<(static_cast<uint32_t>(MASK) >> __builtin_ctz(static_cast<uint32_t>(MASK)))>); \
        template<> struct PeriphRemapField<Zhele::Clock::CLOCK> { using type = CLOCK##Remap; }

    // SPI1_REMAP is contiguous on both (V003 bit0, V00x PCFR1[2:0]).
    ZHELE_CH32_V0_REMAP(Spi1Clock, AFIO_PCFR1_SPI1_REMAP);
#if defined(CH32V00X)
    // Wide contiguous fields.
    ZHELE_CH32_V0_REMAP(I2c1Clock,   AFIO_PCFR1_I2C1_REMAP);
    ZHELE_CH32_V0_REMAP(Usart1Clock, AFIO_PCFR1_USART1_REMAP);
#if defined(AFIO_PCFR1_USART2_REMAP)
    ZHELE_CH32_V0_REMAP(Usart2Clock, AFIO_PCFR1_USART2_REMAP);
#endif
    ZHELE_CH32_V0_REMAP(Tim1Clock,   AFIO_PCFR1_TIM1_REMAP);
    ZHELE_CH32_V0_REMAP(Tim2Clock,   AFIO_PCFR1_TIM2_REMAP);
#else
    // V003: TIM1/TIM2 contiguous; USART1/I2C1 split (low bit + high bit).
    ZHELE_CH32_V0_REMAP(Tim1Clock, AFIO_PCFR1_TIM1_REMAP);
    ZHELE_CH32_V0_REMAP(Tim2Clock, AFIO_PCFR1_TIM2_REMAP);

    template<uint32_t LowMask, uint32_t HighMask>
    struct SplitRemapField
    {
        static void Set(uint32_t value)
        {
            AFIO->PCFR1 = (AFIO->PCFR1 & ~(LowMask | HighMask))
                        | ((value & 1u) ? LowMask  : 0u)
                        | ((value & 2u) ? HighMask : 0u);
        }
        static uint32_t Get()
        {
            return ((AFIO->PCFR1 & LowMask)  ? 1u : 0u)
                 | ((AFIO->PCFR1 & HighMask) ? 2u : 0u);
        }
    };

    template<> struct PeriphRemapField<Zhele::Clock::Usart1Clock>
    { using type = SplitRemapField<AFIO_PCFR1_USART1_REMAP, AFIO_PCFR1_USART1_HIGH_BIT_REMAP>; };
    template<> struct PeriphRemapField<Zhele::Clock::I2c1Clock>
    { using type = SplitRemapField<AFIO_PCFR1_I2C1_REMAP, AFIO_PCFR1_I2C1_HIGH_BIT_REMAP>; };
#endif

    #undef ZHELE_CH32_V0_REMAP

    template<typename Clock>
    using PeriphRemap = typename PeriphRemapField<Clock>::type;
}

#endif // ZHELE_PLATFORM_CH32_V0_AFIO_H
