/**
 * @file
 * Implemets clocks for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_H
#define ZHELE_CLOCK_H

#include <stm32f0xx.h>
#include "../common/clock.h"

namespace Zhele::Clock
{
    #if defined(RCC_HSI48_SUPPORT)
        /**
         * @brief Implements Hsi48 clock source
         */
        IO_REG_WRAPPER(RCC->CR2, RccCr2Reg, uint32_t);
        class Hsi48Clock : public ClockBase<RccCr2Reg>
        {
        public:
            static ClockFrequenceT SrcClockFreq()
            {
                return 48000000;
            }
            static ClockFrequenceT GetDivider() { return 1; }
            static ClockFrequenceT GetMultipler() { return 1; }
            static ClockFrequenceT ClockFreq()
            {
                return SrcClockFreq();
            }
            static bool Enable()
            {
                return ClockBase::EnableClockSource(RCC_CR2_HSI48ON, RCC_CR2_HSI48RDY);
            }
            static bool Disable()
            {
                return ClockBase::DisableClockSource(RCC_CR2_HSI48ON, RCC_CR2_HSI48RDY);
            }
        };
    #endif

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
            ClockFrequenceT clock = SysClock::ClockFreq();
            uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
            uint8_t shiftBits = clockPrescShift[AhbPrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

    const static unsigned ApbPrescalerBitFieldOffset = RCC_CFGR_PPRE_Pos;
    const static unsigned ApbPrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR_PPRE_Msk >> RCC_CFGR_PPRE_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR, ApbPrescalerBitField, uint32_t, ApbPrescalerBitFieldOffset, ApbPrescalerBitFieldLength);
    /**
     * @brief Implements APB clock
     */
    class ApbClock : BusClock<AhbClock, ApbPrescalerBitField>
    {
        using Base = BusClock<AhbClock, ApbPrescalerBitField>;
    public:
        /**
         * @brief APB1 clock prescalers
         */
        enum Prescaler
        {
            Div1 = RCC_CFGR_PPRE_DIV1 >> ApbPrescalerBitFieldOffset, ///< No divide (prescaler = 1)
            Div2 = RCC_CFGR_PPRE_DIV2 >> ApbPrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = RCC_CFGR_PPRE_DIV4 >> ApbPrescalerBitFieldOffset, ///< Prescaler = 4
            Div8 = RCC_CFGR_PPRE_DIV8 >> ApbPrescalerBitFieldOffset, ///< Prescaler = 8
            Div16 = RCC_CFGR_PPRE_DIV16 >> ApbPrescalerBitFieldOffset, ///< Prescaler = 16
        };

        static ClockFrequenceT ClockFreq()
        {
            ClockFrequenceT clock = AhbClock::ClockFreq();
            uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
            uint8_t shiftBits = clockPrescShift[ApbPrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };
    using Apb1Clock = ApbClock;
    using Apb2Clock = ApbClock;

    const static unsigned AdcPrescalerBitFieldOffset = RCC_CFGR_ADCPRE_Pos;
    const static unsigned AdcPrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR_ADCPRE_Msk >> RCC_CFGR_ADCPRE_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR, AdcPrescalerBitField, uint32_t, AdcPrescalerBitFieldOffset, AdcPrescalerBitFieldLength);

    class AdcClockSource : _AdcClockSource
    {
    public:
        /**
         * @brief ADC clock sources
         */
        enum ClockSource
        {
            Apb2 = 0, ///< APB2
        };
        
        /**
         * @brief ADC prescaler
         */
        enum Prescaler
        {
            Div2 = RCC_CFGR_ADCPRE_DIV2 >> AdcPrescalerBitFieldOffset, ///< Prescaler = 2
            Div4 = RCC_CFGR_ADCPRE_DIV4 >> AdcPrescalerBitFieldOffset, ///< Prescaler = 4
        };
        
        /**
         * @brief Select clock source for ADC
         * 
         * @retval true Always
         */
        static bool SelectClockSource(ClockSource source = ClockSource::Apb2)
        {
            return true;
        }
        
        /**
         * @brief Set prescaler for ADC
         * 
         * @par Returns
         *	Nothing
            */
        static void SetPrescaler(Prescaler prescaller)
        {
            AdcPrescalerBitField::Set(static_cast<uint32_t>(prescaller));
        }
        
        /**
         * @brief Returns source clock frequence of ADC
         * 
         * @returns Clock frequence
         */
        static ClockFrequenceT SrcClockFreq()
        {
            return ApbClock::ClockFreq();
        }
        
        /**
         * @brief Returns current clock frequence of ADC
         * 
         * @returns Current frequence
         */
        static ClockFrequenceT ClockFreq()
        {
            return SrcClockFreq() / ((AdcPrescalerBitField::Get() + 1) * 2);
        }
    };
    
    IO_REG_WRAPPER(RCC->AHBENR, AhbClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1ENR, PeriphClockEnable1, uint32_t);
    IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);

    using CrcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_CRCEN, AhbClock>;
    using DmaClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMAEN, AhbClock>;
    using Dma1Clock = DmaClock;
    using FlitfClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FLITFEN, AhbClock>;
    using SramClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_SRAMEN, AhbClock>;
    using PortaClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOAEN, AhbClock>;
    using PortbClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOBEN, AhbClock>;
    using PortcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOCEN, AhbClock>;
    using PortfClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOFEN, AhbClock>;

    using I2c1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C1EN, ApbClock>;
    using PowerClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_PWREN, ApbClock>;
    using Tim3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM3EN, ApbClock>;
    using Tim14Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM14EN, ApbClock>;
    using WatchDogClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGEN, ApbClock>;

    using AdcClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADCEN, AdcClockSource>;
    using DbgMcuClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DBGMCUEN, ApbClock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, ApbClock>;
    using SysCfgCompClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SYSCFGCOMPEN, ApbClock>;
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, ApbClock>;
    using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, ApbClock>;
    using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, ApbClock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, ApbClock>;

    /// Specific *ENR clocks
    #if defined (RCC_AHBENR_DMA2EN)
        using Dma2Clock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA2EN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_GPIODEN)
        using PortdClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIODEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_GPIOEEN)
        using PorteClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOEEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_TSCEN)
        using TscClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_TSCEN, AhbClock>;
    #endif

    #if defined (RCC_APB1ENR_CANEN)
        using CanClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CANEN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_CECEN)
        using CecClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CECEN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_DACEN)
        using DacClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_DACEN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_I2C2EN)
        using I2c2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C2EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_SPI2EN)
        using Spi2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI2EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_TIM2EN)
        using Tim2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM2EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_TIM6EN)
        using Tim6Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM6EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_TIM7EN)
        using Tim7Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM7EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_USART2EN)
        using Usart2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART2EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_USART3EN)
        using Usart3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART3EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_USART4EN)
        using Usart4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART4EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_USART5EN)
        using Usart5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART5EN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_CRSEN)
        using CrsClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CRSEN, ApbClock>;
    #endif
    #if defined (RCC_APB1ENR_USBEN)
        using UsbClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USBEN, ApbClock>;
    #endif

    #if defined (RCC_APB2ENR_TIM15EN)
        using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, ApbClock>;
    #endif
    #if defined (RCC_APB2ENR_USART6EN)
        using Usart6Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART6EN, ApbClock>;
    #endif
    #if defined (RCC_APB2ENR_USART7EN)
        using Usart7Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART7EN, ApbClock>;
    #endif
    #if defined (RCC_APB2ENR_USART8EN)
        using Usart8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART8EN, ApbClock>;
    #endif
}

#endif //! ZHELE_CLOCK_H