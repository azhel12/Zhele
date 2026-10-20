/**
 * @file
 * @brief Implements clocks for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_CLOCK_H
#define ZHELE_PLATFORM_CH32_X0_CLOCK_H

#include <ch32x035.h>

#include "../common/clock.h"

namespace Zhele::Clock
{
    IO_REG_WRAPPER(RCC->APB1PCENR, Apb1PeriphClockEnable, uint32_t);
    IO_REG_WRAPPER(RCC->AHBPCENR,  AhbPeriphClockEnable,  uint32_t);

    class SysClock
    {
    public:
        static constexpr ClockFrequenceT MaxFreq() { return HSI_VALUE; }
        static constexpr ClockFrequenceT ClockFreq() { return HsiClock::ClockFreq(); }
        static constexpr ClockFrequenceT SrcClockFreq() { return ClockFreq(); }
    };

    ZHELE_CH32_DECLARE_CFGR0_BITFIELD(AhbPrescalerBitField, 4, 4);

    class AhbClock : public BusClock<SysClock, AhbPrescalerBitField>
    {
        using Base = BusClock<SysClock, AhbPrescalerBitField>;
    public:
        enum Prescaler
        {
            Div1   = RCC_HPRE_DIV1   >> AhbPrescalerBitFieldOffset,
            Div2   = RCC_HPRE_DIV2   >> AhbPrescalerBitFieldOffset,
            Div3   = RCC_HPRE_DIV3   >> AhbPrescalerBitFieldOffset,
            Div4   = RCC_HPRE_DIV4   >> AhbPrescalerBitFieldOffset,
            Div5   = RCC_HPRE_DIV5   >> AhbPrescalerBitFieldOffset,
            Div6   = RCC_HPRE_DIV6   >> AhbPrescalerBitFieldOffset,
            Div7   = RCC_HPRE_DIV7   >> AhbPrescalerBitFieldOffset,
            Div8   = RCC_HPRE_DIV8   >> AhbPrescalerBitFieldOffset,
            Div16  = RCC_HPRE_DIV16  >> AhbPrescalerBitFieldOffset,
            Div32  = RCC_HPRE_DIV32  >> AhbPrescalerBitFieldOffset,
            Div64  = RCC_HPRE_DIV64  >> AhbPrescalerBitFieldOffset,
            Div128 = RCC_HPRE_DIV128 >> AhbPrescalerBitFieldOffset,
            Div256 = RCC_HPRE_DIV256 >> AhbPrescalerBitFieldOffset
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPresc[16] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
            uint32_t index = AhbPrescalerBitField::Get();
            ClockFrequenceT clock = SysClock::ClockFreq();
            return index < 8 ? (clock / clockPresc[index]) : (clock >> clockPresc[index]);
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Flash::ConfigureFrequence(SysClock::MaxFreq());
            Base::SetPrescaler(prescaler);
            Flash::ConfigureFrequence(ClockFreq());
        }
    };

    class Apb1Clock
    {
    public:
        static ClockFrequenceT SrcClockFreq() { return AhbClock::ClockFreq(); }
        static ClockFrequenceT ClockFreq()    { return AhbClock::ClockFreq(); }
    };

    using Apb2Clock = Apb1Clock;

    template <typename Reg, uint32_t Mask, typename BusClock>
    struct PeriphClock
    {
        static void Enable()  { Reg::Or(Mask); }
        static void Disable() { Reg::And(static_cast<uint32_t>(~Mask)); }

        static ClockFrequenceT ClockFreq() { return BusClock::ClockFreq(); }
    };

    using Usart1Clock = PeriphClock<Apb2PeriphClockEnable, RCC_USART1EN, Apb2Clock>;
    using Spi1Clock   = PeriphClock<Apb2PeriphClockEnable, RCC_SPI1EN,   Apb2Clock>;
    using Tim1Clock   = PeriphClock<Apb2PeriphClockEnable, RCC_TIM1EN,   Apb2Clock>;

    using Usart2Clock = PeriphClock<Apb1PeriphClockEnable, RCC_USART2EN, Apb1Clock>;
    using Usart3Clock = PeriphClock<Apb1PeriphClockEnable, RCC_USART3EN, Apb1Clock>;
    using Usart4Clock = PeriphClock<Apb1PeriphClockEnable, RCC_USART4EN, Apb1Clock>;
    using I2c1Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_I2C1EN,   Apb1Clock>;
    using Tim2Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_TIM2EN,   Apb1Clock>;
    using Tim3Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_TIM3EN,   Apb1Clock>;

    using Dma1Clock   = PeriphClock<AhbPeriphClockEnable,  RCC_DMA1EN,   AhbClock>;
}

#endif // ZHELE_PLATFORM_CH32_X0_CLOCK_H
