/**
 * @file
 * Implements clocks for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_CLOCK_H
#define ZHELE_PLATFORM_STM32_H5_CLOCK_H

#include <stm32h5xx.h>

#include <bit>

// CMSIS for H5 ships no HSI_VALUE (it normally comes from stm32h5xx_hal_conf.h).
// HSI is a 64 MHz RC; the runtime HSIDIV prescaler is applied by HsiClock::ClockFreq().
#if !defined (HSI_VALUE)
    #define HSI_VALUE    ((uint32_t)64000000)
#endif

// For compatibility with "default" CMSIS (F0/F1/F4): system clock mux
#define RCC_CFGR_SW_HSI                    0x00000000U                         /*!< HSI selected as system clock */
#define RCC_CFGR_SW_HSE                    0x00000002U                         /*!< HSE selected as system clock */
#define RCC_CFGR_SW_PLL                    0x00000003U                         /*!< PLL1 selected as system clock */
#define RCC_CFGR_SW                        RCC_CFGR1_SW_Msk                    /*!< System clock switch mask */
#define RCC_CFGR_SWS                       RCC_CFGR1_SWS_Msk                   /*!< System clock switch status mask */
#define RCC_CFGR_SWS_HSI                   0x00000000U                         /*!< HSI oscillator used as system clock */
#define RCC_CFGR_SWS_HSE                   0x00000010U                         /*!< HSE oscillator used as system clock */
#define RCC_CFGR_SWS_PLL                   0x00000018U                         /*!< PLL1 used as system clock */

// The generic PLL code speaks of a single PLL; on H5 that is PLL1.
#define RCC_CR_PLLON                       RCC_CR_PLL1ON
#define RCC_CR_PLLRDY                      RCC_CR_PLL1RDY

// LSI control lives in BDCR on H5, there is no RCC_CSR at all.
#define RCC_CSR_LSION                      RCC_BDCR_LSION
#define RCC_CSR_LSIRDY                     RCC_BDCR_LSIRDY

#include "../common/clock.h"

namespace Zhele::Clock
{
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CR, HsiPrescaler, RCC_CR_HSIDIV);

    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1CFGR, Pll1Source, RCC_PLL1CFGR_PLL1SRC);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1CFGR, Pll1InputRange, RCC_PLL1CFGR_PLL1RGE);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1CFGR, PllM, RCC_PLL1CFGR_PLL1M);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1DIVR, PllN, RCC_PLL1DIVR_PLL1N);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1DIVR, PllP, RCC_PLL1DIVR_PLL1P);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1DIVR, PllQ, RCC_PLL1DIVR_PLL1Q);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->PLL1DIVR, PllR, RCC_PLL1DIVR_PLL1R);

    /**
     * @brief HSI divider (HSIDIV), applied between the 64 MHz RC and hsi_ck
     */
    inline unsigned HsiClock::GetDivider()
    {
        return 1u << HsiPrescaler::Get();
    }

    inline ClockFrequenceT HsiClock::ClockFreq()
    {
        return SrcClockFreq() / GetDivider();
    }

    /**
     * @brief Set HSI divider
     *
     * @tparam divider Divider value (1, 2, 4 or 8)
     *
     * @par Returns
     *  Nothing
     */
    template<unsigned divider>
    inline void SetHsiDivider()
    {
        static_assert(divider == 1 || divider == 2 || divider == 4 || divider == 8, "Invalid HSI divider value!");
        HsiPrescaler::Set(std::countr_zero(divider));
        while ((RCC->CR & RCC_CR_HSIDIVF) == 0)
            ;
    }

    /**
     * @brief Implements the 4 MHz low-power internal oscillator (CSI)
     */
    class CsiClock : public ClockBase<>
    {
    public:
        static constexpr ClockFrequenceT SrcClockFreq() { return 4000000; }
        static constexpr unsigned GetDivider() { return 1; }
        static constexpr unsigned GetMultipler() { return 1; }
        static constexpr ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

        static bool Enable() { return ClockBase::EnableClockSource(RCC_CR_CSION, RCC_CR_CSIRDY); }
        static bool Disable() { return ClockBase::DisableClockSource(RCC_CR_CSION, RCC_CR_CSIRDY); }
    };

    /**
     * @brief Implements the 48 MHz internal oscillator (HSI48), the USB/RNG kernel source
     */
    class Hsi48Clock : public ClockBase<>
    {
    public:
        static constexpr ClockFrequenceT SrcClockFreq() { return 48000000; }
        static constexpr unsigned GetDivider() { return 1; }
        static constexpr unsigned GetMultipler() { return 1; }
        static constexpr ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

        static bool Enable() { return ClockBase::EnableClockSource(RCC_CR_HSI48ON, RCC_CR_HSI48RDY); }
        static bool Disable() { return ClockBase::DisableClockSource(RCC_CR_HSI48ON, RCC_CR_HSI48RDY); }
    };

    /**
     * @brief Implements the external low speed oscillator (LSE)
     */
    class LseClock : public ClockBase<RccBdcrReg>
    {
    public:
        static constexpr ClockFrequenceT SrcClockFreq() { return 32768; }
        static constexpr unsigned GetDivider() { return 1; }
        static constexpr unsigned GetMultipler() { return 1; }
        static constexpr ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

        static bool Enable() { return ClockBase::EnableClockSource(RCC_BDCR_LSEON, RCC_BDCR_LSERDY); }
        static bool Disable() { return ClockBase::DisableClockSource(RCC_BDCR_LSEON, RCC_BDCR_LSERDY); }
    };

    /**
     * @brief PLL1 reference (ref1_ck) frequency range
     *
     * @details
     * PLL1RGE must match the frequency seen at the PLL input, that is
     * source frequency / PLL1M. Programming it is mandatory on H5.
     */
    enum class PllInputRange
    {
        Range1To2Mhz = 0b00,
        Range2To4Mhz = 0b01,
        Range4To8Mhz = 0b10,
        Range8To16Mhz = 0b11
    };

    /**
     * @brief Set PLL1 input frequency range (PLL1RGE)
     *
     * @param [in] range Range which contains source frequency / PLL1M
     *
     * @par Returns
     *  Nothing
     */
    inline void SetPllInputRange(PllInputRange range)
    {
        Pll1InputRange::Set(static_cast<uint32_t>(range));
    }

    /**
     * @brief Select PLL1 input range from the reference frequency itself
     *
     * @param [in] refClockFreq PLL input frequency (source / PLL1M), in Hz
     *
     * @par Returns
     *  Nothing
     */
    inline void ConfigurePllInputRange(ClockFrequenceT refClockFreq)
    {
        PllInputRange range = refClockFreq >= 8000000
            ? PllInputRange::Range8To16Mhz
            : refClockFreq >= 4000000
                ? PllInputRange::Range4To8Mhz
                : refClockFreq >= 2000000
                    ? PllInputRange::Range2To4Mhz
                    : PllInputRange::Range1To2Mhz;

        SetPllInputRange(range);
    }

    inline unsigned PllClock::GetDivider()
    {
        return PllM::Get();
    }

    template<unsigned divider>
    inline void PllClock::SetDivider()
    {
        static_assert(1 <= divider && divider <= PllM::MaxValue, "Invalid divider value!");
        PllM::Set(divider);
        // PLL1RGE must match the resulting reference frequency; getting it wrong
        // is silent, so keep it in sync automatically. Call SelectClockSource()
        // before SetDivider() so the source frequency is already known.
        ConfigurePllInputRange(SrcClockFreq() / divider);
    }

    inline unsigned PllClock::GetMultipler()
    {
        return PllN::Get() + 1;
    }

    template<unsigned multiplier>
    inline void PllClock::SetMultiplier()
    {
        static_assert(4 <= multiplier && multiplier <= 512, "Invalid multiplier value!");
        PllN::Set(multiplier - 1);
    }

    template<PllClock::ClockSource clockSource>
    inline void PllClock::SelectClockSource()
    {
        Pll1Source::Set(clockSource == External
            ? 0b11u  // HSE
            : 0b01u); // HSI
    }

    inline PllClock::ClockSource PllClock::GetClockSource()
    {
        return Pll1Source::Get() == 0b11u
            ? ClockSource::External
            : ClockSource::Internal;
    }

    inline unsigned PllClock::GetSystemOutputDivider()
    {
        return PllP::Get() + 1;
    }

    template<unsigned divider>
    inline void PllClock::SetSystemOutputDivider()
    {
        static_assert(2 <= divider && divider <= (PllP::MaxValue + 1), "Invalid divider value!");
        static_assert(divider % 2 == 0, "PLL1P does not allow odd division factors!");
        RCC->PLL1CFGR |= RCC_PLL1CFGR_PLL1PEN;
        PllP::Set(divider - 1);
    }

    inline unsigned PllClock::GetUsbOutputDivider()
    {
        return PllQ::Get() + 1;
    }

    template<unsigned divider>
    inline void PllClock::SetUsbOutputDivider()
    {
        static_assert(1 <= divider && divider <= (PllQ::MaxValue + 1), "Invalid divider value!");
        RCC->PLL1CFGR |= RCC_PLL1CFGR_PLL1QEN;
        PllQ::Set(divider - 1);
    }

    inline unsigned PllClock::GetI2SOutputDivider()
    {
        return PllR::Get() + 1;
    }

    template<unsigned divider>
    inline void PllClock::SetI2SOutputDivider()
    {
        static_assert(1 <= divider && divider <= (PllR::MaxValue + 1), "Invalid divider value!");
        RCC->PLL1CFGR |= RCC_PLL1CFGR_PLL1REN;
        PllR::Set(divider - 1);
    }

    /**
     * @brief Core voltage scaling (PWR_VOSCR.VOS)
     *
     * @details
     * The maximum system frequency and the required flash latency both depend on
     * it: VOS3 (reset value) tops out at 100 MHz, VOS0 is needed for 250 MHz.
     */
    enum class VoltageScale
    {
        Scale3 = 0b00, ///< Up to 100 MHz (default after reset)
        Scale2 = 0b01, ///< Up to 150 MHz
        Scale1 = 0b10, ///< Up to 200 MHz
        Scale0 = 0b11  ///< Up to 250 MHz
    };

    /**
     * @brief Set core voltage scaling and wait for the regulator
     *
     * @details
     * Must be called (with a high enough scale) *before* raising the system
     * frequency. Levels are stepped through one by one, as the reference manual
     * requires.
     *
     * @param [in] scale Target voltage scale
     *
     * @par Returns
     *  Nothing
     */
    inline void SetVoltageScale(VoltageScale scale)
    {
        auto target = static_cast<uint32_t>(scale);
        auto current = (PWR->VOSCR & PWR_VOSCR_VOS_Msk) >> PWR_VOSCR_VOS_Pos;

        while (current != target)
        {
            current = current < target ? current + 1 : current - 1;
            PWR->VOSCR = (PWR->VOSCR & ~PWR_VOSCR_VOS_Msk) | (current << PWR_VOSCR_VOS_Pos);
            while ((PWR->VOSSR & PWR_VOSSR_VOSRDY) == 0)
                ;
        }
    }

    /**
     * @brief Returns currently applied voltage scale
     *
     * @returns Voltage scale
     */
    inline VoltageScale GetVoltageScale()
    {
        return static_cast<VoltageScale>((PWR->VOSCR & PWR_VOSCR_VOS_Msk) >> PWR_VOSCR_VOS_Pos);
    }

    /**
     * @brief Kernel clock source of the SPI peripherals
     *
     * @details
     * SPI on H5 is *not* fed from the APB bus clock: the reset default is
     * pll1_q_ck, and PCLK is not even an option.
     */
    enum class SpiKernelClockSource
    {
        Pll1Q = 0b000,  ///< pll1_q_ck (default after reset)
        Pll2P = 0b001,  ///< pll2_p_ck
        AudioClk = 0b011, ///< external AUDIOCLK pin
        PerCk = 0b100   ///< per_ck (HSI/CSI/HSE, selected by CKPERSEL)
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CCIPR3, Spi1KernelClockSelect, RCC_CCIPR3_SPI1SEL);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CCIPR3, Spi2KernelClockSelect, RCC_CCIPR3_SPI2SEL);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CCIPR3, Spi3KernelClockSelect, RCC_CCIPR3_SPI3SEL);

    /**
     * @brief Select the kernel clock of one SPI
     *
     * @tparam _Select Spi1/Spi2/Spi3KernelClockSelect bitfield wrapper
     *
     * @param [in] source Kernel clock source
     *
     * @par Returns
     *  Nothing
     */
    template<typename _Select>
    inline void SelectSpiKernelClock(SpiKernelClockSource source)
    {
        _Select::Set(static_cast<uint32_t>(source));
    }

    /**
     * @brief Source of per_ck, the common "peripheral clock" (RCC_CCIPR5.CKPERSEL)
     */
    enum class PeripheralClockSource
    {
        Hsi = 0b00,  ///< hsi_ker_ck (default after reset)
        Csi = 0b01,  ///< csi_ker_ck
        Hse = 0b10   ///< hse_ck
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CCIPR5, PeripheralClockSelect, RCC_CCIPR5_CKERPSEL);

    /**
     * @brief Select the source of per_ck
     *
     * @param [in] source Clock source
     *
     * @par Returns
     *  Nothing
     */
    inline void SelectPeripheralClock(PeripheralClockSource source)
    {
        PeripheralClockSelect::Set(static_cast<uint32_t>(source));
    }

    const static unsigned AhbPrescalerBitFieldOffset = RCC_CFGR2_HPRE_Pos;
    const static unsigned AhbPrescalerBitFieldLength = GetBitFieldLength<(RCC_CFGR2_HPRE_Msk >> RCC_CFGR2_HPRE_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->CFGR2, AhbPrescalerBitField, uint32_t, AhbPrescalerBitFieldOffset, AhbPrescalerBitFieldLength);

    /**
     * @brief Implements AHB clock (rcc_hclk)
     */
    class AhbClock : public BusClock<SysClock, AhbPrescalerBitField>
    {
        using Base = BusClock<SysClock, AhbPrescalerBitField>;
    public:
        /// AHB prescaler values
        enum Prescaler
        {
            Div1 = 0b0000, ///< No divide (prescaler = 1)
            Div2 = 0b1000, ///< Prescaler = 2
            Div4 = 0b1001, ///< Prescaler = 4
            Div8 = 0b1010, ///< Prescaler = 8
            Div16 = 0b1011, ///< Prescaler = 16
            Div64 = 0b1100, ///< Prescaler = 64
            Div128 = 0b1101, ///< Prescaler = 128
            Div256 = 0b1110, ///< Prescaler = 256
            Div512 = 0b1111 ///< Prescaler = 512
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

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR2, Apb1PrescalerBitField, RCC_CFGR2_PPRE1);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR2, Apb2PrescalerBitField, RCC_CFGR2_PPRE2);
    DECLARE_IO_BITFIELD_WRAPPER(RCC->CFGR2, Apb3PrescalerBitField, RCC_CFGR2_PPRE3);

    /**
     * @brief Implements an APB bus clock
     *
     * @tparam _PrescalerBitField PPRE1/PPRE2/PPRE3 bitfield wrapper
     */
    template<typename _PrescalerBitField>
    class ApbClockBase : BusClock<AhbClock, _PrescalerBitField>
    {
        using Base = BusClock<AhbClock, _PrescalerBitField>;
    public:
        /// APB clock prescalers
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
            uint8_t shiftBits = clockPrescShift[_PrescalerBitField::Get()];
            clock >>= shiftBits;
            return clock;
        }

        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            Base::SetPrescaler(prescaler);
        }

        static void SetPrescaler(Prescaler prescaler)
        {
            Base::SetPrescaler(prescaler);
        }
    };

    using Apb1Clock = ApbClockBase<Apb1PrescalerBitField>;
    using Apb2Clock = ApbClockBase<Apb2PrescalerBitField>;
    using Apb3Clock = ApbClockBase<Apb3PrescalerBitField>;
    /// Most generic code says just "ApbClock" and means the low-speed bus
    using ApbClock = Apb1Clock;

    IO_REG_WRAPPER(RCC->AHB1ENR, Ahb1ClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB2ENR, Ahb2ClockEnableReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1LENR, PeriphClockEnable1L, uint32_t);
    IO_REG_WRAPPER(RCC->APB1HENR, PeriphClockEnable1H, uint32_t);
    IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);
    IO_REG_WRAPPER(RCC->APB3ENR, PeriphClockEnable3, uint32_t);

    IO_REG_WRAPPER(RCC->AHB1RSTR, Ahb1ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->AHB2RSTR, Ahb2ResetReg, uint32_t);
    IO_REG_WRAPPER(RCC->APB1LRSTR, ApbResetReg1L, uint32_t);
    IO_REG_WRAPPER(RCC->APB1HRSTR, ApbResetReg1H, uint32_t);
    IO_REG_WRAPPER(RCC->APB2RSTR, ApbResetReg2, uint32_t);
    IO_REG_WRAPPER(RCC->APB3RSTR, ApbResetReg3, uint32_t);

    // GPIO ports live on AHB2 on H5
    using PortaClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOAEN, AhbClock>;
    using PortbClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOBEN, AhbClock>;
    using PortcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOCEN, AhbClock>;
    using PortdClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIODEN, AhbClock>;
    using PorthClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOHEN, AhbClock>;
#if defined (RCC_AHB2ENR_GPIOEEN)
    using PorteClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOEEN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_GPIOFEN)
    using PortfClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOFEN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_GPIOGEN)
    using PortgClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_GPIOGEN, AhbClock>;
#endif

    // AHB1
    using Dma1Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPDMA1EN, AhbClock>;
    using Dma2Clock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_GPDMA2EN, AhbClock>;
    using DmaClock = Dma1Clock;
    using GpDma1Clock = Dma1Clock;
    using GpDma2Clock = Dma2Clock;
    using FlashClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_FLITFEN, AhbClock>;
    using CrcClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_CRCEN, AhbClock>;
    using RamCfgClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_RAMCFGEN, AhbClock>;
    using BackupRamClock = ClockControl<Ahb1ClockEnableReg, RCC_AHB1ENR_BKPRAMEN, AhbClock>;

    // AHB2
    using AdcClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_ADCEN, AhbClock>;
    using Adc1Clock = AdcClock;
    using Dac1Clock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_DAC1EN, AhbClock>;
    using DacClock = Dac1Clock;
    using RngClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_RNGEN, AhbClock>;
#if defined (RCC_AHB2ENR_HASHEN)
    using HashClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_HASHEN, AhbClock>;
#endif
#if defined (RCC_AHB2ENR_PKAEN)
    using PkaClock = ClockControl<Ahb2ClockEnableReg, RCC_AHB2ENR_PKAEN, AhbClock>;
#endif

    // APB1 (low word)
    using Tim2Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_TIM2EN, Apb1Clock>;
    using Tim3Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_TIM3EN, Apb1Clock>;
    using Tim6Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_TIM6EN, Apb1Clock>;
    using Tim7Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_TIM7EN, Apb1Clock>;
    using WatchDogClock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_WWDGEN, Apb1Clock>;
    using Spi2Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_SPI2EN, Apb1Clock>;
    using Spi3Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_SPI3EN, Apb1Clock>;
    using Usart2Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_USART2EN, Apb1Clock>;
    using Usart3Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_USART3EN, Apb1Clock>;
    using I2c1Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_I2C1EN, Apb1Clock>;
    using I2c2Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_I2C2EN, Apb1Clock>;
    using I3c1Clock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_I3C1EN, Apb1Clock>;
    using CrsClock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_CRSEN, Apb1Clock>;
#if defined (RCC_APB1LENR_OPAMPEN)
    using OpampClock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_OPAMPEN, Apb1Clock>;
#endif
#if defined (RCC_APB1LENR_COMPEN)
    using CompClock = ClockControl<PeriphClockEnable1L, RCC_APB1LENR_COMPEN, Apb1Clock>;
#endif

    // APB1 (high word)
#if defined (RCC_APB1HENR_FDCANEN)
    using FdCanClock = ClockControl<PeriphClockEnable1H, RCC_APB1HENR_FDCANEN, Apb1Clock>;
#endif
#if defined (RCC_APB1HENR_LPTIM2EN)
    using LpTim2Clock = ClockControl<PeriphClockEnable1H, RCC_APB1HENR_LPTIM2EN, Apb1Clock>;
#endif
#if defined (RCC_APB1HENR_DTSEN)
    using DtsClock = ClockControl<PeriphClockEnable1H, RCC_APB1HENR_DTSEN, Apb1Clock>;
#endif

    // APB2
    using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
    using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
    using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;
#if defined (RCC_APB2ENR_USBEN)
    using UsbClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USBEN, Apb2Clock>;
#endif

    // APB3
    using SysCfgClock = ClockControl<PeriphClockEnable3, RCC_APB3ENR_SBSEN, Apb3Clock>;
    using SbsClock = SysCfgClock;
    using RtcClock = ClockControl<PeriphClockEnable3, RCC_APB3ENR_RTCAPBEN, Apb3Clock>;
    using LpUart1Clock = ClockControl<PeriphClockEnable3, RCC_APB3ENR_LPUART1EN, Apb3Clock>;
    using LpTim1Clock = ClockControl<PeriphClockEnable3, RCC_APB3ENR_LPTIM1EN, Apb3Clock>;
#if defined (RCC_APB3ENR_I3C2EN)
    using I3c2Clock = ClockControl<PeriphClockEnable3, RCC_APB3ENR_I3C2EN, Apb3Clock>;
#endif
} // namespace Zhele::Clock

#endif //! ZHELE_PLATFORM_STM32_H5_CLOCK_H
