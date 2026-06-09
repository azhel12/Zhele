/**
 * @file
 * Implements clocks for stm32c0 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_C0_CLOCK_H
#define ZHELE_PLATFORM_STM32_C0_CLOCK_H

#include <stm32c0xx.h>

#include <bit>

// CMSIS for C0 does not expose HSI_VALUE: the internal oscillator is the fixed 48 MHz
// HSI48. HSISYS (and therefore the reported clock) is HSI48 / HSIDIV, see HsiClock below.
#ifndef HSI_VALUE
    #define HSI_VALUE ((uint32_t)48000000)
#endif

// CMSIS for C0 only exposes the raw RCC_CFGR_SW_x / RCC_CFGR_SWS_x bits, so provide the
// named values the portable clock code expects (mirrors what g0/clock.h does).
#ifndef RCC_CFGR_SW_HSI
    #define RCC_CFGR_SW_HSI                0x00000000U     /*!< HSISYS selected as system clock */
#endif
#ifndef RCC_CFGR_SW_HSE
    #define RCC_CFGR_SW_HSE                0x00000001U     /*!< HSE selected as system clock */
#endif
#ifndef RCC_CFGR_SWS_HSI
    #define RCC_CFGR_SWS_HSI              0x00000000U      /*!< HSISYS used as system clock */
#endif
#ifndef RCC_CFGR_SWS_HSE
    #define RCC_CFGR_SWS_HSE             0x00000008U       /*!< HSE used as system clock */
#endif

#include "../common/clock.h"

namespace Zhele::Clock
{
    // --- HSI48 / HSISYS -----------------------------------------------------------------
    // HSI_VALUE (from CMSIS) is the undivided HSI48 frequency (48 MHz). The actual SYSCLK
    // source HSISYS is HSI48 / HSIDIV, where HSIDIV encodes 2^n (n in [0..7] => /1../128).
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CR, HsiDivBitField, RCC_CR_HSIDIV);

    inline unsigned HsiClock::GetDivider()
    {
        return 1u << HsiDivBitField::Get();
    }

    inline ClockFrequenceT HsiClock::ClockFreq()
    {
        return HsiClock::SrcClockFreq() / HsiClock::GetDivider();
    }

    /**
     * @brief Set the HSISYS prescaler (HSI48 -> SYSCLK divider)
     *
     * @tparam divider Power-of-two divider in [1 .. 128]. Use 1 for the full 48 MHz.
     */
    template<unsigned divider>
    inline void SetHsiSysDivider()
    {
        static_assert(divider == 1 || divider == 2 || divider == 4 || divider == 8
            || divider == 16 || divider == 32 || divider == 64 || divider == 128,
            "HSIDIV must be a power of two in the range [1 .. 128]");
        HsiDivBitField::Set(std::countr_zero(divider));
    }

    // --- AHB / APB buses ----------------------------------------------------------------
    const static unsigned AhbPrescalerBitFieldOffset = RCC_CFGR_HPRE_Pos;
    const static unsigned AhbPrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR_HPRE_Msk >> RCC_CFGR_HPRE_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR, AhbPrescalerBitField, uint32_t, AhbPrescalerBitFieldOffset, AhbPrescalerBitFieldLength);

    class AhbClock : public BusClock<SysClock, AhbPrescalerBitField>
    {
        using Base = BusClock<SysClock, AhbPrescalerBitField>;
    public:
        // AHB prescaler values
        enum Prescaler
        {
            Div1 = 0b0000 >> AhbPrescalerBitFieldOffset, ///< No divide (prescaler = 1)
            Div2 = 0b1000 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = 0b1001 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 4
            Div8 = 0b1010 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 8
            Div16 = 0b1011 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 16
            Div64 = 0b1100 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 64
            Div128 = 0b1101 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 128
            Div256 = 0b1110 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 256
            Div512 = 0b1111 >> AhbPrescalerBitFieldOffset ///< Prescaler = 512
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

            ClockFrequenceT clock = SysClock::ClockFreq();
            uint8_t shiftBits = clockPrescShift[AhbPrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR, ApbPrescalerBitField, RCC_CFGR_PPRE);

    /**
     * @brief Implements APB clock
     */
    class ApbClock : BusClock<AhbClock, ApbPrescalerBitField>
    {
        using Base = BusClock<AhbClock, ApbPrescalerBitField>;
    public:
        /**
         * @brief APB clock prescalers
         */
        enum Prescaler
        {
            Div1 = 0b000, ///< No divide (prescaler = 1)
            Div2 = 0b100, ///< Prescaler = 2
            Div4 = 0b101, ///< Prescaler = 4
            Div8 = 0b110, ///< Prescaler = 8
            Div16 = 0b111, ///< Prescaler = 16
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};

            ClockFrequenceT clock = AhbClock::ClockFreq();
            uint8_t shiftBits = clockPrescShift[ApbPrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Base::SetPrescaler(prescaler);
        }
    };
    using Apb1Clock = ApbClock;
    using Apb2Clock = ApbClock;

    // --- Peripheral clock enable / reset registers --------------------------------------
    IO_REG_WRAPPER(RCC->AHBENR, AhbClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->APBENR1, PeriphClockEnable1, uint32_t);
    IO_REG_WRAPPER(RCC->APBENR2, PeriphClockEnable2, uint32_t);
    IO_REG_WRAPPER(RCC->IOPENR, IOPeriphClockEnable, uint32_t);

    IO_REG_WRAPPER(RCC->AHBRSTR, AhbResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APBRSTR1, ApbResetReg1, uint32_t);
    IO_REG_WRAPPER(RCC->APBRSTR2, ApbResetReg2, uint32_t);

    // --- GPIO port clocks (guarded so the same file fits every C0 part) -----------------
#if defined (RCC_IOPENR_GPIOAEN)
    using PortaClock = ClockControl<IOPeriphClockEnable, RCC_IOPENR_GPIOAEN, ApbClock>;
#endif
#if defined (RCC_IOPENR_GPIOBEN)
    using PortbClock = ClockControl<IOPeriphClockEnable, RCC_IOPENR_GPIOBEN, ApbClock>;
#endif
#if defined (RCC_IOPENR_GPIOCEN)
    using PortcClock = ClockControl<IOPeriphClockEnable, RCC_IOPENR_GPIOCEN, ApbClock>;
#endif
#if defined (RCC_IOPENR_GPIODEN)
    using PortdClock = ClockControl<IOPeriphClockEnable, RCC_IOPENR_GPIODEN, ApbClock>;
#endif
#if defined (RCC_IOPENR_GPIOFEN)
    using PortfClock = ClockControl<IOPeriphClockEnable, RCC_IOPENR_GPIOFEN, ApbClock>;
#endif

    // --- Peripheral clocks --------------------------------------------------------------
#if defined (RCC_AHBENR_DMA1EN)
    using DmaClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA1EN, AhbClock>;
    using Dma1Clock = DmaClock;
#endif
#if defined (RCC_AHBENR_FLASHEN)
    using FlashClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FLASHEN, AhbClock>;
#endif
#if defined (RCC_AHBENR_CRCEN)
    using CrcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_CRCEN, AhbClock>;
#endif

#if defined (RCC_APBENR1_TIM2EN)
    using Tim2Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_TIM2EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_TIM3EN)
    using Tim3Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_TIM3EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_RTCAPBEN)
    using RtcClock = ClockControl<PeriphClockEnable1, RCC_APBENR1_RTCAPBEN, ApbClock>;
#endif
#if defined (RCC_APBENR1_WWDGEN)
    using WatchDogClock = ClockControl<PeriphClockEnable1, RCC_APBENR1_WWDGEN, ApbClock>;
#endif
#if defined (RCC_APBENR1_SPI2EN)
    using Spi2Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_SPI2EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_USART2EN)
    using Usart2Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_USART2EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_USART3EN)
    using Usart3Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_USART3EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_USART4EN)
    using Usart4Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_USART4EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_I2C1EN)
    using I2c1Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_I2C1EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_I2C2EN)
    using I2c2Clock = ClockControl<PeriphClockEnable1, RCC_APBENR1_I2C2EN, ApbClock>;
#endif
#if defined (RCC_APBENR1_DBGEN)
    using DbgClock = ClockControl<PeriphClockEnable1, RCC_APBENR1_DBGEN, ApbClock>;
#endif
#if defined (RCC_APBENR1_PWREN)
    using PowerClock = ClockControl<PeriphClockEnable1, RCC_APBENR1_PWREN, ApbClock>;
#endif

#if defined (RCC_APBENR2_SYSCFGEN)
    using SysCfgClock = ClockControl<PeriphClockEnable2, RCC_APBENR2_SYSCFGEN, ApbClock>;
#endif
#if defined (RCC_APBENR2_TIM1EN)
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_TIM1EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_SPI1EN)
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_SPI1EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_USART1EN)
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_USART1EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_TIM14EN)
    using Tim14Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_TIM14EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_TIM15EN)
    using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_TIM15EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_TIM16EN)
    using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_TIM16EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_TIM17EN)
    using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APBENR2_TIM17EN, ApbClock>;
#endif
#if defined (RCC_APBENR2_ADCEN)
    using AdcClock = ClockControl<PeriphClockEnable2, RCC_APBENR2_ADCEN, ApbClock>;
#endif
} // namespace Zhele::Clock

#endif //! ZHELE_PLATFORM_STM32_C0_CLOCK_H
