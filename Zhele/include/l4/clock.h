/**
 * @file
 * Implemets clocks for stm32f4 series
 * 
 * @author Alexey Zhelonkin (based on Konstantin Chizhov)
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_H
#define ZHELE_CLOCK_H

#include <stm32l4xx.h>
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
     * @brief Implements APB clock
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
     * @brief Implements APB clock
     */
    class Apb2Clock : BusClock<AhbClock, Apb2PrescalerBitField>
    {
        using Base = BusClock<AhbClock, Apb2PrescalerBitField>;
    public:
        /**
         * @brief APB1 clock prescalers
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
            uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
            uint8_t shiftBits = clockPrescShift[Apb2PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        static void SetPrescaler(Prescaler prescaler)
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

    IO_REG_WRAPPER(RCC->APB1RSTR1, Apb11ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1RSTR2, Apb12ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB2RSTR, Apb2ResetReg, uint32_t);

    using Dma1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA1EN, AhbClock>;
    using Dma2Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA2EN, AhbClock>;
    using FlashClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_FLASHEN, AhbClock>;
    using CrcClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CRCEN, AhbClock>;
    using TscClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_TSCEN, AhbClock>;

    using PortaClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOAEN, AhbClock>;
    using PortbClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOBEN, AhbClock>;
    using PortcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOCEN, AhbClock>;
    using PorthClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOHEN, AhbClock>;
    using AdcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_ADCEN, AhbClock>;
    using RngClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_RNGEN, AhbClock>; 

    using Tim2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM2EN, Apb1Clock>;
    using Tim6Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM6EN, Apb1Clock>;

    using Usart2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USART2EN, Apb1Clock>;
    using WatchDogClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_WWDGEN, Apb1Clock>;
    using I2c1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C1EN, Apb1Clock>;
    using I2c3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C3EN, Apb1Clock>;
    using PwrClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_PWREN, Apb1Clock>;
    using OpampClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_OPAMPEN, Apb1Clock>;
    using LPTim1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_LPTIM1EN, Apb1Clock>;
    using LpUart1Clock = ClockControl<PeriphClockEnable12, RCC_APB1ENR2_LPUART1EN, Apb1Clock>;
    using LpTim2Clock = ClockControl<PeriphClockEnable12, RCC_APB1ENR2_LPTIM2EN, Apb1Clock>;

    using SysCfgCompClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SYSCFGEN, Apb2Clock>;
    using FirewallClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_FWEN, Apb2Clock>;
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;
    using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, Apb2Clock>;
    using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, Apb2Clock>;

    #if defined (RCC_AHB1ENR_DMA2DEN)
        using Dma2DClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA2DEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_DMAMUX1EN)
        using DmaMux1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMAMUX1EN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_GPIOIEN)
        using PortiClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOIEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_DCMIEN)
        using DcmiClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DCMIEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_HASHEN)
        using HashClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_HASHEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_OSPIMEN)
        using OSPIMClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_OSPIMEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_SDMMC1EN)
        using SdMmc1Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_SDMMC1EN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_SDMMC2EN)
        using SdMmc2Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_SDMMC2EN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_GPIODEN)
        using PortdClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIODEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_GPIOEEN)
        using PorteClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOEEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_GPIOFEN)
        using PortfClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOFEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_GPIOGEN)
        using PortgClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOGEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_AESEN)
        using AesClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_AESEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_OTGFSEN)
        using OtgFsClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_OTGFSEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_PKAEN)
        using PKAClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_PKAEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GFXMMUEN)
        using GFXMMUClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GFXMMUEN, AhbClock>;
    #endif
    #if defined (RCC_AHB3ENR_FMCEN)
        using FmcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB3ENR_FMCEN, AhbClock>;
    #endif
    #if defined (RCC_AHB3ENR_OSPI1EN)
        using OSPI1Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB3ENR_OSPI1EN, AhbClock>;
    #endif
    #if defined (RCC_AHB3ENR_OSPI2EN)
        using OSPI2Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB3ENR_OSPI2EN, AhbClock>;
    #endif    
    #if defined (RCC_AHB3ENR_QSPIEN)
        using QSPIClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB3ENR_QSPIEN, AhbClock>;
    #endif

    #if defined (RCC_APB1ENR1_RTCAPBEN)
        using RtcApbClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_RTCAPBEN, Apb1Clock>;
    #endif    
    #if defined (RCC_APB1ENR1_SPI2EN)
        using Spi2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_SPI2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_USART3EN)
        using Usart3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USART3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_I2C2EN)
        using I2c2Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_I2C2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_CRSEN)
        using CrsClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_CRSEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_USBFSEN)
        using UsbFsClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_USBFSEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_TIM7EN)
        using TIM7Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM7EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_SPI3EN)
        using Spi3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_SPI3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_CAN1EN)
        using Can1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_CAN1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_DAC1EN)
        using Dac1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_DAC1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR2_SWPMI1EN)
        using Swpmi1Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR2_SWPMI1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_LCDEN)
        using LcdClock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_LCDEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_TIM3EN)
        using Tim3Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_UART4EN)
        using Uart4Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_UART4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR2_I2C4EN)
        using I2c4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR2_I2C4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_TIM4EN)
        using Tim4Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_TIM5EN)
        using Tim5Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_TIM5EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR1_UART5EN)
        using Uart5Clock = ClockControl<PeriphClockEnable11, RCC_APB1ENR1_UART5EN, Apb1Clock>;
    #endif

    #if defined (RCC_APB2ENR_SDMMC1EN)
        using SdMmc1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SDMMC1EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SAI1EN)
        using Sai1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SAI1EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_DFSDM1EN)
        using Dfsdm1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DFSDM1EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM8EN)
        using Tim8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM17EN)
        using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SAI2EN)
        using Sai2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SAI2EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_LTDCEN)
        using LTDCClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_LTDCEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_DSIEN)
        using DSIClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DSIEN, Apb2Clock>;
    #endif
}
#endif //! ZHELE_CLOCK_H