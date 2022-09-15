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

#include <stm32f4xx.h>
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
            static const uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
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
            static const uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
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
    
    IO_REG_WRAPPER(RCC->APB1ENR, PeriphClockEnable1, uint32_t);
    IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);

    IO_REG_WRAPPER(RCC->APB1RSTR, Apb1ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB2RSTR, Apb2ResetReg, uint32_t);

    using PortaClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOAEN, AhbClock>;
    using PortbClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOBEN, AhbClock>;
    using PortcClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOCEN, AhbClock>;
    using PorthClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOHEN, AhbClock>;
    using CrcClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CRCEN, AhbClock>;
    using Dma1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA1EN, AhbClock>;
    using Dma2Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA2EN, AhbClock>;

    using I2c1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C1EN, Apb1Clock>;
    using I2c2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C2EN, Apb1Clock>;
    using PwrClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_PWREN, Apb1Clock>;
    using Tim5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM5EN, Apb1Clock>;
    using Usart2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART2EN, Apb1Clock>;
    using WatchDogClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGEN, Apb1Clock>;

    using Adc1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC1EN, Apb2Clock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
    using SysCfgCompClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SYSCFGEN, Apb2Clock>;
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
    using Tim9Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM9EN, Apb2Clock>;
    using Tim11Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM11EN, Apb2Clock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;

    #if defined (RCC_AHB1ENR_BKPSRAMEN)
        using BackupSramClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_BKPSRAMEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_CCMDATARAMEN)
        using CcmDataRamClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CCMDATARAMEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_DMA2DEN)
        using Dma2Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_DMA2DEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_ETHMACEN)
        using EthMacClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_ETHMACEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_ETHMACPTPEN)
        using EthMacPtpClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_ETHMACPTPEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_ETHMACRXEN)
        using EthMacRxClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_ETHMACRXEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_ETHMACTXEN)
        using EthMacTxClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_ETHMACTXEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIODEN)
        using PortdClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIODEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOEEN)
        using PorteClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOEEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOFEN)
        using PortfClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOFEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOGEN)
        using PortgClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOGEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOIEN)
        using PortiClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOIEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOJEN)
        using PortjClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOJEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_GPIOKEN)
        using PortkClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPIOKEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_OTGHSEN)
        using OtgHsClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_OTGHSEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_OTGHSULPIEN)
        using OtgHsUlpiClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_OTGHSULPIEN, AhbClock>;
    #endif
    #if defined (RCC_AHB1ENR_RNGEN)
        using RngClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_RNGEN, AhbClock>;
    #endif
/* Low-power. Not implemented yet
    #if defined (RCC_AHB1LPENR_BKPSRAMLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_BKPSRAMLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_DMA2DLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_DMA2DLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_ETHMACLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_ETHMACLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_ETHMACPTPLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_ETHMACPTPLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_ETHMACRXLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_ETHMACRXLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_ETHMACTXLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_ETHMACTXLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIODLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIODLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOELPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOELPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOFLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOFLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOGLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOGLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOILPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOILPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOJLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOJLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_GPIOKLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_GPIOKLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_OTGHSLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_OTGHSLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_OTGHSULPILPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_OTGHSULPILPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_RNGLPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_RNGLPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_SRAM2LPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_SRAM2LPEN, >;
    #endif
    #if defined (RCC_AHB1LPENR_SRAM3LPEN)
        using EditMe = ClockControl<, RCC_AHB1LPENR_SRAM3LPEN, >;
    #endif
*/
    #if defined (RCC_AHB2ENR_AESEN)
        using AesClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_AESEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_CRYPEN)
        using CrypClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_CRYPEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_DCMIEN)
        using DcmiClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DCMIEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_HASHEN)
        using HashClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_HASHEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_OTGFSEN)
        using OtgFsClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_OTGFSEN, AhbClock>;
    #endif
    #if defined (RCC_AHB2ENR_RNGEN)
        using RngClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_RNGEN, AhbClock>;
    #endif
/* Low-power. Not implemented yet
    #if defined (RCC_AHB2LPENR_AESLPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_AESLPEN, >;
    #endif
    #if defined (RCC_AHB2LPENR_CRYPLPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_CRYPLPEN, >;
    #endif
    #if defined (RCC_AHB2LPENR_DCMILPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_DCMILPEN, >;
    #endif
    #if defined (RCC_AHB2LPENR_HASHLPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_HASHLPEN, >;
    #endif
    #if defined (RCC_AHB2LPENR_OTGFSLPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_OTGFSLPEN, >;
    #endif
    #if defined (RCC_AHB2LPENR_RNGLPEN)
        using EditMe = ClockControl<, RCC_AHB2LPENR_RNGLPEN, >;
    #endif
*/
    #if defined (RCC_AHB3ENR_FMCEN)
        using FmcClock = ClockControl<Ahb3ClockEnableReg, RCC_AHB3ENR_FMCEN, AhbClock>;
    #endif
    #if defined (RCC_AHB3ENR_FSMCEN)
        using FsmcClock = ClockControl<Ahb3ClockEnableReg, RCC_AHB3ENR_FSMCEN, AhbClock>;
    #endif
    #if defined (RCC_AHB3ENR_QSPIEN)
        using QSpiClock = ClockControl<Ahb3ClockEnableReg, RCC_AHB3ENR_QSPIEN, AhbClock>;
    #endif
/* Low-power. Not implemented yet
    #if defined (RCC_AHB3LPENR_FMCLPEN)
        using EditMe = ClockControl<, RCC_AHB3LPENR_FMCLPEN, >;
    #endif
    #if defined (RCC_AHB3LPENR_FSMCLPEN)
        using EditMe = ClockControl<, RCC_AHB3LPENR_FSMCLPEN, >;
    #endif
    #if defined (RCC_AHB3LPENR_QSPILPEN)
        using EditMe = ClockControl<, RCC_AHB3LPENR_QSPILPEN, >;
    #endif
*/
    #if defined (RCC_APB1ENR_CAN1EN)
        using Can1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_CAN2EN)
        using Can2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_CAN3EN)
        using Can3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_CECEN)
        using CecClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CECEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_DACEN)
        using DacClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_DACEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_FMPI2C1EN)
        using FmpI2c1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_FMPI2C1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_I2C3EN)
        using I2c3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_LPTIM1EN)
        using LpTim1 = ClockControl<PeriphClockEnable1, RCC_APB1ENR_LPTIM1EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_RTCAPBEN)
        using RtcApb = ClockControl<PeriphClockEnable1, RCC_APB1ENR_RTCAPBEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_SPDIFRXEN)
        using SpdifRx = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPDIFRXEN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_SPI2EN)
        using Spi2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_SPI3EN)
        using Spi3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI3EN, Apb1Clock>;
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
    #if defined (RCC_APB1ENR_TIM2EN)
        using Tim2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM2EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM3EN)
        using Tim3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM3EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM4EN)
        using Tim4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM6EN)
        using Tim6Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM6EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_TIM7EN)
        using Tim7Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM7EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART4EN)
        using Uart4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART4EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART5EN)
        using Uart5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART5EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART7EN)
        using Uart7Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART7EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_UART8EN)
        using Uart8Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART8EN, Apb1Clock>;
    #endif
    #if defined (RCC_APB1ENR_USART3EN)
        using Usart3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART3EN, Apb1Clock>;
    #endif
/* Low-power. Not implemented yet
    #if defined (RCC_APB1LPENR_CAN1LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_CAN1LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_CAN2LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_CAN2LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_CAN3LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_CAN3LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_CECLPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_CECLPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_DACLPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_DACLPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_FMPI2C1LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_FMPI2C1LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_I2C3LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_I2C3LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_LPTIM1LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_LPTIM1LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_RTCAPBLPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_RTCAPBLPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_SPDIFRXLPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_SPDIFRXLPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_SPI2LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_SPI2LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_SPI3LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_SPI3LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM12LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM12LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM13LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM13LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM14LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM14LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM2LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM2LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM3LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM3LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM4LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM4LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM6LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM6LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_TIM7LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_TIM7LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_UART4LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_UART4LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_UART5LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_UART5LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_UART7LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_UART7LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_UART8LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_UART8LPEN, >;
    #endif
    #if defined (RCC_APB1LPENR_USART3LPEN)
        using EditMe = ClockControl<, RCC_APB1LPENR_USART3LPEN, >;
    #endif
*/
    #if defined (RCC_APB2ENR_ADC2EN)
        using Adc2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC2EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_ADC3EN)
        using Adc3Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC3EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_DFSDM1EN)
        using Dfsdm1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DFSDM1EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_DFSDM2EN)
        using Dfsdm2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DFSDM2EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_DSIEN)
        using DsiClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_DSIEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_EXTITEN)
        using ExtiClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_EXTITEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_LTDCEN)
        using LtdcClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_LTDCEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SAI1EN)
        using Sai1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SAI1EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SAI2EN)
        using Sai2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SAI2EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SDIOEN)
        using SdioClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SDIOEN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SPI4EN)
        using Spi4Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI4EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SPI5EN)
        using Spi5Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI5EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_SPI6EN)
        using Spi6Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI6EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM10EN)
        using Tim10Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM10EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_TIM8EN)
        using Tim8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_UART10EN)
        using Uart10Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_UART10EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_UART9EN)
        using Uart9Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_UART9EN, Apb2Clock>;
    #endif
    #if defined (RCC_APB2ENR_USART6EN)
        using Usart6Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART6EN, Apb2Clock>;
    #endif
/* Low-power. Not implemented yet
    #if defined (RCC_APB2LPENR_ADC2LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_ADC2LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_ADC3LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_ADC3LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_DFSDM1LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_DFSDM1LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_DFSDM2LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_DFSDM2LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_DSILPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_DSILPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_EXTITLPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_EXTITLPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_LTDCLPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_LTDCLPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SAI1LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SAI1LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SAI2LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SAI2LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SDIOLPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SDIOLPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SPI4LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SPI4LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SPI5LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SPI5LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_SPI6LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_SPI6LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_TIM10LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_TIM10LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_TIM8LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_TIM8LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_UART10LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_UART10LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_UART9LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_UART9LPEN, >;
    #endif
    #if defined (RCC_APB2LPENR_USART6LPEN)
        using EditMe = ClockControl<, RCC_APB2LPENR_USART6LPEN, >;
    #endif
*/
/* Not implemented
    #if defined (RCC_CKGATENR_AHB2APB1_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_AHB2APB1_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_AHB2APB2_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_AHB2APB2_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_CM4DBG_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_CM4DBG_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_FLITF_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_FLITF_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_RCC_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_RCC_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_SPARE_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_SPARE_CKEN, >;
    #endif
    #if defined (RCC_CKGATENR_SRAM_CKEN)
        using EditMe = ClockControl<, RCC_CKGATENR_SRAM_CKEN, >;
    #endif
*/
}
#endif //! ZHELE_CLOCK_H