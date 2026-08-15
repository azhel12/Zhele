/**
 * @file
 * Implemets clocks for stm32g4 series
 *
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_CLOCK_H
#define ZHELE_PLATFORM_STM32_G4_CLOCK_H

#include <stm32g4xx.h>

#include "../common/clock.h"

namespace Zhele::Clock
{
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllM, RCC_PLLCFGR_PLLM);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllN, RCC_PLLCFGR_PLLN);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllP, RCC_PLLCFGR_PLLP);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllPDiv, RCC_PLLCFGR_PLLPDIV);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllQ, RCC_PLLCFGR_PLLQ);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllR, RCC_PLLCFGR_PLLR);

    inline unsigned PllClock::GetDivider()
    {
        return PllM::Get() + 1;
    }

    template<unsigned divider>
    inline void PllClock::SetDivider()
    {
        static_assert(1 <= divider && divider <= (PllM::MaxValue + 1), "Invalid divider value!");
        PllM::Set(divider - 1);
    }

    inline unsigned PllClock::GetMultipler()
    {
        return PllN::Get();
    }

    template<unsigned multiplier>
    inline void PllClock::SetMultiplier()
    {
        static_assert(8 <= multiplier && multiplier <= 127, "Invalid multiplier value!");
        PllN::Set(multiplier);
    }

    template<PllClock::ClockSource clockSource>
    inline void PllClock::SelectClockSource()
    {
        RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC_Msk))
            | (clockSource == External
                ? RCC_PLLCFGR_PLLSRC_HSE
                : RCC_PLLCFGR_PLLSRC_HSI);
    }

    inline PllClock::ClockSource PllClock::GetClockSource()
    {
        return (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC_Msk) == RCC_PLLCFGR_PLLSRC_HSE
            ? ClockSource::External
            : ClockSource::Internal;
    }

    /**
     * @brief System clock is taken from PLLR output (2/4/6/8)
     */
    inline unsigned PllClock::GetSystemOutputDivider()
    {
        return (PllR::Get() + 1) * 2;
    }

    template<unsigned divider>
    inline void PllClock::SetSystemOutputDivider()
    {
        static_assert(divider == 2 || divider == 4 || divider == 6 || divider == 8, "Invalid divider value!");
        RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
        PllR::Set(divider / 2 - 1);
    }

    /**
     * @brief 48 MHz clock (USB, RNG, ...) is taken from PLLQ output (2/4/6/8)
     */
    inline unsigned PllClock::GetUsbOutputDivider()
    {
        return (PllQ::Get() + 1) * 2;
    }

    template<unsigned divider>
    inline void PllClock::SetUsbOutputDivider()
    {
        static_assert(divider == 2 || divider == 4 || divider == 6 || divider == 8, "Invalid divider value!");
        RCC->PLLCFGR |= RCC_PLLCFGR_PLLQEN;
        PllQ::Set(divider / 2 - 1);
    }

    /**
     * @brief ADC/SAI clock is taken from PLLP output (PLLPDIV = 2..31)
     */
    inline unsigned PllClock::GetI2SOutputDivider()
    {
        return PllPDiv::Get() != 0
            ? PllPDiv::Get()
            : (PllP::Get() != 0 ? 17 : 7);
    }

    template<unsigned divider>
    inline void PllClock::SetI2SOutputDivider()
    {
        static_assert(2 <= divider && divider <= PllPDiv::MaxValue, "Invalid divider value!");
        RCC->PLLCFGR |= RCC_PLLCFGR_PLLPEN;
        PllPDiv::Set(divider);
    }

    IO_REG_WRAPPER(RCC->CRRCR, RccCrrcrReg, uint32_t);

    /**
     * @brief Implements HSI48 (RC 48 MHz) clock source (USB, RNG)
     */
    class Hsi48Clock : public ClockBase<RccCrrcrReg>
    {
        using Base = ClockBase<RccCrrcrReg>;
    public:
        static constexpr ClockFrequenceT SrcClockFreq()
        {
            return 48000000u;
        }

        static constexpr unsigned GetDivider() { return 1; }

        static constexpr unsigned GetMultipler() { return 1; }

        static constexpr ClockFrequenceT ClockFreq()
        {
            return SrcClockFreq();
        }

        static bool Enable()
        {
            return Base::EnableClockSource(RCC_CRRCR_HSI48ON, RCC_CRRCR_HSI48RDY);
        }

        static bool Disable()
        {
            return Base::DisableClockSource(RCC_CRRCR_HSI48ON, RCC_CRRCR_HSI48RDY);
        }
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR, AhbPrescalerBitField, RCC_CFGR_HPRE);

    class AhbClock : public BusClock<SysClock, AhbPrescalerBitField>
    {
        using Base = BusClock<SysClock, AhbPrescalerBitField>;
    public:
        // AHB prescaler values
        enum Prescaler
        {
            Div1 = RCC_CFGR_HPRE_DIV1 >> AhbPrescalerBitFieldOffset, ///< No divide (prescaler = 1)
            Div2 = RCC_CFGR_HPRE_DIV2 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = RCC_CFGR_HPRE_DIV4 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 4
            Div8 = RCC_CFGR_HPRE_DIV8 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 8
            Div16 = RCC_CFGR_HPRE_DIV16 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 16
            Div64 = RCC_CFGR_HPRE_DIV64 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 64
            Div128 = RCC_CFGR_HPRE_DIV128 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 128
            Div256 = RCC_CFGR_HPRE_DIV256 >> AhbPrescalerBitFieldOffset, ///< Prescaler = 256
            Div512 = RCC_CFGR_HPRE_DIV512 >> AhbPrescalerBitFieldOffset ///< Prescaler = 512
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

            ClockFrequenceT clock = SysClock::ClockFreq();
            uint8_t shiftBits = clockPrescShift[AhbPrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Base::SetPrescaler(prescaler);
        }
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR, Apb1PrescalerBitField, RCC_CFGR_PPRE1);

    /**
     * @brief Implements APB1 clock
     */
    class Apb1Clock : BusClock<AhbClock, Apb1PrescalerBitField>
    {
        using Base = BusClock<AhbClock, Apb1PrescalerBitField>;
    public:
        /**
         * @brief APB1 clock prescalers
         */
        enum Prescaler
        {
            Div1 = RCC_CFGR_PPRE1_DIV1 >> Apb1PrescalerBitFieldOffset, ///< No divide (prescaler = 1)
            Div2 = RCC_CFGR_PPRE1_DIV2 >> Apb1PrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = RCC_CFGR_PPRE1_DIV4 >> Apb1PrescalerBitFieldOffset, ///< Prescaler = 4
            Div8 = RCC_CFGR_PPRE1_DIV8 >> Apb1PrescalerBitFieldOffset, ///< Prescaler = 8
            Div16 = RCC_CFGR_PPRE1_DIV16 >> Apb1PrescalerBitFieldOffset, ///< Prescaler = 16
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};

            ClockFrequenceT clock = AhbClock::ClockFreq();
            uint8_t shiftBits = clockPrescShift[Apb1PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Base::SetPrescaler(prescaler);
        }
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR, Apb2PrescalerBitField, RCC_CFGR_PPRE2);

    /**
     * @brief Implements APB2 clock
     */
    class Apb2Clock : BusClock<AhbClock, Apb2PrescalerBitField>
    {
        using Base = BusClock<AhbClock, Apb2PrescalerBitField>;
    public:
        /**
         * @brief APB2 clock prescalers
         */
        enum Prescaler
        {
            Div1 = RCC_CFGR_PPRE2_DIV1 >> Apb2PrescalerBitFieldOffset, ///< No divide (prescaler = 1)
            Div2 = RCC_CFGR_PPRE2_DIV2 >> Apb2PrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = RCC_CFGR_PPRE2_DIV4 >> Apb2PrescalerBitFieldOffset, ///< Prescaler = 4
            Div8 = RCC_CFGR_PPRE2_DIV8 >> Apb2PrescalerBitFieldOffset, ///< Prescaler = 8
            Div16 = RCC_CFGR_PPRE2_DIV16 >> Apb2PrescalerBitFieldOffset, ///< Prescaler = 16
        };

        static ClockFrequenceT ClockFreq()
        {
            static constexpr uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};

            ClockFrequenceT clock = AhbClock::ClockFreq();
            uint8_t shiftBits = clockPrescShift[Apb2PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Base::SetPrescaler(prescaler);
        }
    };

    IO_REG_WRAPPER(RCC->AHB1ENR, Ahb1ClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB2ENR, Ahb2ClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB3ENR, Ahb3ClockEnableReg, uint32_t);

    IO_REG_WRAPPER(RCC->APB1ENR1, PeriphClockEnable11, uint32_t);
    IO_REG_WRAPPER(RCC->APB1ENR2, PeriphClockEnable12, uint32_t);
    IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);

    IO_REG_WRAPPER(RCC->AHB1RSTR, Ahb1ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB2RSTR, Ahb2ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB3RSTR, Ahb3ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1RSTR1, Apb11ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1RSTR2, Apb12ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB2RSTR, Apb2ResetReg, uint32_t);

    // AHB1
    using Dma1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA1EN, AhbClock>;
    using Dma2Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA2EN, AhbClock>;
    using DmaMux1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMAMUX1EN, AhbClock>;
    using CordicClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CORDICEN, AhbClock>;
    using FmacClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_FMACEN, AhbClock>;
    using FlashClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_FLASHEN, AhbClock>;
    using CrcClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CRCEN, AhbClock>;

    // AHB2
    using PortaClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOAEN, AhbClock>;
    using PortbClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOBEN, AhbClock>;
    using PortcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOCEN, AhbClock>;
    using PortdClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIODEN, AhbClock>;
    using PorteClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOEEN, AhbClock>;
    using PortfClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOFEN, AhbClock>;
    using PortgClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOGEN, AhbClock>;
    using Adc12Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_ADC12EN, AhbClock>;
    using Dac1Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DAC1EN, AhbClock>;
    using DacClock = Dac1Clock;
    using Dac3Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DAC3EN, AhbClock>;
    using RngClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_RNGEN, AhbClock>;
#if defined (RCC_AHB2ENR_ADC345EN)
    using Adc345Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_ADC345EN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_DAC2EN)
    using Dac2Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DAC2EN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_DAC4EN)
    using Dac4Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DAC4EN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_AESEN)
    using AesClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_AESEN, AhbClock>;
#endif

    // AHB3
#if defined (RCC_AHB3ENR_FMCEN)
    using FmcClock = ClockControl<Ahb3ClockEnableReg, RCC_AHB3ENR_FMCEN, AhbClock>;
#endif
#if defined (RCC_AHB3ENR_QSPIEN)
    using QspiClock = ClockControl<Ahb3ClockEnableReg, RCC_AHB3ENR_QSPIEN, AhbClock>;
#endif

    // APB1 (ENR1)
    using Tim2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM2EN, Apb1Clock>;
    using Tim3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM3EN, Apb1Clock>;
    using Tim4Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM4EN, Apb1Clock>;
    using Tim6Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM6EN, Apb1Clock>;
    using Tim7Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM7EN, Apb1Clock>;
    using CrsClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_CRSEN, Apb1Clock>;
    using RtcClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_RTCAPBEN, Apb1Clock>;
    using WatchDogClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_WWDGEN, Apb1Clock>;
    using Spi2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_SPI2EN, Apb1Clock>;
    using Spi3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_SPI3EN, Apb1Clock>;
    using Usart2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USART2EN, Apb1Clock>;
    using Usart3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USART3EN, Apb1Clock>;
    using Uart4Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_UART4EN, Apb1Clock>;
    using I2c1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C1EN, Apb1Clock>;
    using I2c2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C2EN, Apb1Clock>;
    using UsbClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USBEN, Apb1Clock>;
    using FdCanClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_FDCANEN, Apb1Clock>;
    using PowerClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_PWREN, Apb1Clock>;
    using PwrClock = PowerClock;
    using I2c3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C3EN, Apb1Clock>;
    using LpTim1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_LPTIM1EN, Apb1Clock>;
#if defined (RCC_APB1ENR1_TIM5EN)
    using Tim5Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM5EN, Apb1Clock>;
#endif
#if defined (RCC_APB1ENR1_UART5EN)
    using Uart5Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_UART5EN, Apb1Clock>;
#endif

    // APB1 (ENR2)
    using LpUart1Clock = ClockControl<PeriphClockEnable12, RCC_APB1ENR2_LPUART1EN, Apb1Clock>;
    using Ucpd1Clock = ClockControl<PeriphClockEnable12, RCC_APB1ENR2_UCPD1EN, Apb1Clock>;
#if defined (RCC_APB1ENR2_I2C4EN)
    using I2c4Clock = ClockControl<PeriphClockEnable12, RCC_APB1ENR2_I2C4EN, Apb1Clock>;
#endif

    // APB2
    using SysCfgClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SYSCFGEN, Apb2Clock>;
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
    using Tim8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;
    using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, Apb2Clock>;
    using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, Apb2Clock>;
    using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, Apb2Clock>;
    using Sai1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SAI1EN, Apb2Clock>;
#if defined (RCC_APB2ENR_SPI4EN)
    using Spi4Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI4EN, Apb2Clock>;
#endif
#if defined (RCC_APB2ENR_TIM20EN)
    using Tim20Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM20EN, Apb2Clock>;
#endif
#if defined (RCC_APB2ENR_HRTIM1EN)
    using Hrtim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_HRTIM1EN, Apb2Clock>;
#endif

    /**
     * @brief Power control helpers (voltage scaling range 1 boost mode is required for SYSCLK > 150 MHz)
     */
    class Power
    {
    public:
        /**
         * @brief Enables range 1 boost mode (R1MODE = 0)
         *
         * @details
         * Must be called before switching SYSCLK above 150 MHz.
         *
         * @par Returns
         *  Nothing
         */
        static void EnableBoostMode()
        {
            PowerClock::Enable();
            PWR->CR5 &= ~PWR_CR5_R1MODE;
        }

        /**
         * @brief Disables boost mode (R1MODE = 1, range 1 normal mode)
         *
         * @par Returns
         *  Nothing
         */
        static void DisableBoostMode()
        {
            PowerClock::Enable();
            PWR->CR5 |= PWR_CR5_R1MODE;
        }

        /**
         * @brief Returns true if boost mode is enabled
         */
        static bool IsBoostModeEnabled()
        {
            return (PWR->CR5 & PWR_CR5_R1MODE) == 0;
        }
    };
} // namespace Zhele::Clock

#endif //! ZHELE_PLATFORM_STM32_G4_CLOCK_H
