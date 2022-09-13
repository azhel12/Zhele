/**
 * @file
 * Implemets clocks for stm32f1 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_H
#define ZHELE_CLOCK_H

#include <stm32f1xx.h>
#include "../common/clock.h"

namespace Zhele::Clock
{
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


    const static unsigned Apb1PrescalerBitFieldOffset = RCC_CFGR_PPRE1_Pos;
    const static unsigned Apb1PrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR_PPRE1_Msk >> RCC_CFGR_PPRE1_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR, Apb1PrescalerBitField, uint32_t, Apb1PrescalerBitFieldOffset, Apb1PrescalerBitFieldLength);
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
            ClockFrequenceT clock = AhbClock::ClockFreq();
            uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
            uint8_t shiftBits = clockPrescShift[Apb1PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

    const static unsigned Apb2PrescalerBitFieldOffset = RCC_CFGR_PPRE2_Pos;
    const static unsigned Apb2PrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR_PPRE2_Msk >> RCC_CFGR_PPRE2_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR, Apb2PrescalerBitField, uint32_t, Apb2PrescalerBitFieldOffset, Apb2PrescalerBitFieldLength);
    /**
     * @brief Implements APB2 clock
     */
    class Apb2Clock : public BusClock<AhbClock, Apb2PrescalerBitField>
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
            ClockFrequenceT clock = AhbClock::ClockFreq();
            const uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
            uint8_t shiftBits = clockPrescShift[Apb2PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

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
            Div6 = RCC_CFGR_ADCPRE_DIV6 >> AdcPrescalerBitFieldOffset, ///< Prescaler = 6
            Div8 = RCC_CFGR_ADCPRE_DIV8 >> AdcPrescalerBitFieldOffset, ///< Prescaler = 8
        };
        
        /**
         * @brief Select clock source for ADC (like Belarus presidential election)
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
            AdcPrescalerBitField::Set((uint32_t)prescaller);
        }
        
        /**
         * @brief Returns source clock frequence of ADC
         * 
         * @returns Clock frequence
         */
        static ClockFrequenceT SrcClockFreq()
        {
            return Apb2Clock::ClockFreq();
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
    using Dma1Clock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA1EN, AhbClock>;
    using FlitfClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FLITFEN, AhbClock>;
    using SramClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_SRAMEN, AhbClock>;

    using BackupClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_BKPEN, Apb1Clock>;
    using I2c1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C1EN, Apb1Clock>;
    using PowerClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_PWREN, Apb1Clock>;
    using Tim2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM2EN, Apb1Clock>;
    using Tim3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM3EN, Apb1Clock>;
    using Usart2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART2EN, Apb1Clock>;
    using WatchDogClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGEN, Apb1Clock>;

    using AfioClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_AFIOEN, Apb2Clock>;
    using Adc1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC1EN, AdcClockSource>;
    using PortaClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPAEN, Apb2Clock>;
    using PortbClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPBEN, Apb2Clock>;
    using PortcClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPCEN, Apb2Clock>;
    using PortdClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPDEN, Apb2Clock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;

    /// Specific *ENR clocks
    #if defined (RCC_AHBENR_DMA2EN)
        using Dma2Clock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA2EN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_ETHMACEN)
        using EthernetClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_ETHMACRXEN)
        using EthernetRxClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACRXEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_ETHMACTXEN)
        using EthernetTxClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACTXEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_FSMCEN)
        using FsmcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FSMCEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_OTGFSEN)
        using OtgFsClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_OTGFSEN, AhbClock>;
    #endif
    #if defined (RCC_AHBENR_SDIOEN)
        using SdioClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_SDIOEN, AhbClock>;
    #endif

    #if defined (RCC_APB1ENR_CAN1EN)
        using Can1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_CAN2EN)
        using Can2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_CECEN)
        using CecClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CECEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_DACEN)
        using DacClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_DACEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_I2C2EN)
        using I2c2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_SPI2EN)
        using Spi2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_SPI3EN)
        using Spi3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM4EN)
        using Tim4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM5EN)
        using Tim5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM5EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM6EN)
        using Tim6Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM6EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM7EN)
        using Tim7Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM7EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM12EN)
        using Tim12Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM12EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM13EN)
        using Tim13Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM13EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM14EN)
        using Tim14Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM14EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_USART3EN)
        using Usart3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART4EN)
        using Uart4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART5EN)
        using Uart5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART5EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_USBEN)
        using UsbClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USBEN, Apb1Clock>;
    #endif

    #if defined (RCC_APB2ENR_ADC2EN)
        using Adc2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC2EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_ADC3EN)
        using Adc3Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC3EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_IOPEEN)
        using PorteClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPEEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_IOPFEN)
        using PortfClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPFEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_IOPGEN)
        using PortgClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPGEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM8EN)
        using Tim8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM9EN)
        using Tim9Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM9EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM10EN)
        using Tim10Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM10EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM11EN)
        using Tim11Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM11EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM15EN)
        using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM16EN)
        using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM17EN)
        using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, Apb2Clock>;
    #endif

}

#endif //! ZHELE_CLOCK_H