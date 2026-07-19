/**
 * @file
 * CH32V20x clocks: port and peripheral clocks plus the PLL/bus clock tree.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_CLOCK_H
#define ZHELE_PLATFORM_CH32_V2_CLOCK_H

#include <ch32v20x.h>

// ch32v20x.h omits the E/F/G clock-enable bits; supply the SPL values.
#ifndef RCC_IOPEEN
#  define RCC_IOPEEN    0x00000040  // RCC_APB2Periph_GPIOE
#endif
#ifndef RCC_IOPFEN
#  define RCC_IOPFEN    0x00000080  // RCC_APB2Periph_GPIOF
#endif
#ifndef RCC_IOPGEN
#  define RCC_IOPGEN    0x00000100  // RCC_APB2Periph_GPIOG
#endif

#include "../common/clock.h"

// Some peripheral enable bits are absent from ch32v20x.h; supply the SPL values.
#ifndef RCC_TIM4EN
#  define RCC_TIM4EN    0x00000004  // RCC_APB1Periph_TIM4
#endif
#ifndef RCC_SPI2EN
#  define RCC_SPI2EN    0x00004000  // RCC_APB1Periph_SPI2
#endif
#ifndef RCC_USART3EN
#  define RCC_USART3EN  0x00040000  // RCC_APB1Periph_USART3
#endif
#ifndef RCC_I2C2EN
#  define RCC_I2C2EN    0x00400000  // RCC_APB1Periph_I2C2
#endif

namespace Zhele::Clock
{
  IO_REG_WRAPPER(RCC->APB1PCENR, Apb1PeriphClockEnable, uint32_t);
  IO_REG_WRAPPER(RCC->AHBPCENR,  AhbPeriphClockEnable,  uint32_t);

  inline ClockFrequenceT PllClock::SrcClockFreq()
  {
    // TODO/verify: assumes CH32V20x_D6 (HSE direct, HSI/2 to PLL); D8/D8W differ
    return GetClockSource() == External ? HSE_VALUE : (HSI_VALUE >> 1);
  }

  inline unsigned PllClock::GetDivider()
  {
    // TODO/verify: PLLXTPRE (HSE/2 pre-divider) not modeled
    return 1;
  }

  inline unsigned PllClock::GetMultipler()
  {
    unsigned mul = ((RCC->CFGR0 & RCC_PLLMULL) >> 18) + 2;
    if (mul == 17)
      mul = 18; // field 0xF encodes x18
    return mul;
  }

  template<unsigned multiplier>
  inline void PllClock::SetMultiplier()
  {
    static_assert((multiplier >= 2 && multiplier <= 16) || multiplier == 18,
                  "CH32V203 PLL multiplier must be 2..16 or 18");
    constexpr uint32_t field = (multiplier == 18) ? 0xFu : (multiplier - 2);
    RCC->CFGR0 = (RCC->CFGR0 & ~static_cast<uint32_t>(RCC_PLLMULL)) | (field << 18);
  }

  template<PllClock::ClockSource clockSource>
  inline void PllClock::SelectClockSource()
  {
    RCC->CFGR0 = clockSource == External
      ? (RCC->CFGR0 | RCC_PLLSRC)
      : (RCC->CFGR0 & ~static_cast<uint32_t>(RCC_PLLSRC));
  }

  inline PllClock::ClockSource PllClock::GetClockSource()
  {
    return (RCC->CFGR0 & RCC_PLLSRC) ? External : Internal;
  }

  inline ClockFrequenceT PllClock::ClockFreq()
  {
    return SrcClockFreq() * GetMultipler();
  }

  ZHELE_CH32_DECLARE_CFGR0_BITFIELD(AhbPrescalerBitField, 4, 4);

  /**
   * @brief AHB (HCLK) clock.
   */
  class AhbClock : public BusClock<SysClock, AhbPrescalerBitField>
  {
    using Base = BusClock<SysClock, AhbPrescalerBitField>;
  public:
    enum Prescaler
    {
      Div1   = RCC_HPRE_DIV1   >> AhbPrescalerBitFieldOffset,
      Div2   = RCC_HPRE_DIV2   >> AhbPrescalerBitFieldOffset,
      Div4   = RCC_HPRE_DIV4   >> AhbPrescalerBitFieldOffset,
      Div8   = RCC_HPRE_DIV8   >> AhbPrescalerBitFieldOffset,
      Div16  = RCC_HPRE_DIV16  >> AhbPrescalerBitFieldOffset,
      Div64  = RCC_HPRE_DIV64  >> AhbPrescalerBitFieldOffset,
      Div128 = RCC_HPRE_DIV128 >> AhbPrescalerBitFieldOffset,
      Div256 = RCC_HPRE_DIV256 >> AhbPrescalerBitFieldOffset,
      Div512 = RCC_HPRE_DIV512 >> AhbPrescalerBitFieldOffset
    };

    static ClockFrequenceT ClockFreq()
    {
      static constexpr uint8_t clockPrescShift[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
      return SysClock::ClockFreq() >> clockPrescShift[AhbPrescalerBitField::Get()];
    }

    template<Prescaler prescaler>
    static void SetPrescaler()
    {
      Base::SetPrescaler(prescaler);
    }
  };

  ZHELE_CH32_DECLARE_CFGR0_BITFIELD(Apb1PrescalerBitField, 8, 3);

  /**
   * @brief APB1 (PCLK1) clock.
   */
  class Apb1Clock : public BusClock<AhbClock, Apb1PrescalerBitField>
  {
    using Base = BusClock<AhbClock, Apb1PrescalerBitField>;
  public:
    enum Prescaler
    {
      Div1  = RCC_PPRE1_DIV1  >> Apb1PrescalerBitFieldOffset,
      Div2  = RCC_PPRE1_DIV2  >> Apb1PrescalerBitFieldOffset,
      Div4  = RCC_PPRE1_DIV4  >> Apb1PrescalerBitFieldOffset,
      Div8  = RCC_PPRE1_DIV8  >> Apb1PrescalerBitFieldOffset,
      Div16 = RCC_PPRE1_DIV16 >> Apb1PrescalerBitFieldOffset
    };

    static ClockFrequenceT ClockFreq()
    {
      static constexpr uint8_t clockPrescShift[8] = {0, 0, 0, 0, 1, 2, 3, 4};
      return AhbClock::ClockFreq() >> clockPrescShift[Apb1PrescalerBitField::Get()];
    }

    template<Prescaler prescaler>
    static void SetPrescaler()
    {
      Base::SetPrescaler(prescaler);
    }
  };

  ZHELE_CH32_DECLARE_CFGR0_BITFIELD(Apb2PrescalerBitField, 11, 3);

  /**
   * @brief APB2 (PCLK2) clock.
   */
  class Apb2Clock : public BusClock<AhbClock, Apb2PrescalerBitField>
  {
    using Base = BusClock<AhbClock, Apb2PrescalerBitField>;
  public:
    enum Prescaler
    {
      Div1  = RCC_PPRE2_DIV1  >> Apb2PrescalerBitFieldOffset,
      Div2  = RCC_PPRE2_DIV2  >> Apb2PrescalerBitFieldOffset,
      Div4  = RCC_PPRE2_DIV4  >> Apb2PrescalerBitFieldOffset,
      Div8  = RCC_PPRE2_DIV8  >> Apb2PrescalerBitFieldOffset,
      Div16 = RCC_PPRE2_DIV16 >> Apb2PrescalerBitFieldOffset
    };

    static ClockFrequenceT ClockFreq()
    {
      static constexpr uint8_t clockPrescShift[8] = {0, 0, 0, 0, 1, 2, 3, 4};
      return AhbClock::ClockFreq() >> clockPrescShift[Apb2PrescalerBitField::Get()];
    }

    template<Prescaler prescaler>
    static void SetPrescaler()
    {
      Base::SetPrescaler(prescaler);
    }
  };

  inline ClockFrequenceT SysClock::MaxFreq() { return 144000000u; }

  /**
   * @brief Peripheral clock control; ClockFreq() reads its bus clock live.
   */
  template <typename Reg, uint32_t Mask, typename BusClock>
  struct PeriphClock
  {
    static void Enable()  { Reg::Or(Mask); }
    static void Disable() { Reg::And(static_cast<uint32_t>(~Mask)); }

    static ClockFrequenceT ClockFreq() { return BusClock::ClockFreq(); }
  };

  // APB2 peripherals
  using Usart1Clock = PeriphClock<Apb2PeriphClockEnable, RCC_USART1EN, Apb2Clock>;
  using Spi1Clock   = PeriphClock<Apb2PeriphClockEnable, RCC_SPI1EN,   Apb2Clock>;
  using Tim1Clock   = PeriphClock<Apb2PeriphClockEnable, RCC_TIM1EN,   Apb2Clock>;

  // APB1 peripherals
  using Usart2Clock = PeriphClock<Apb1PeriphClockEnable, RCC_USART2EN, Apb1Clock>;
  using Usart3Clock = PeriphClock<Apb1PeriphClockEnable, RCC_USART3EN, Apb1Clock>;
  using I2c1Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_I2C1EN,   Apb1Clock>;
  using I2c2Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_I2C2EN,   Apb1Clock>;
  using Spi2Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_SPI2EN,   Apb1Clock>;
  using Tim2Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_TIM2EN,   Apb1Clock>;
  using Tim3Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_TIM3EN,   Apb1Clock>;
  using Tim4Clock   = PeriphClock<Apb1PeriphClockEnable, RCC_TIM4EN,   Apb1Clock>;

  // AHB peripherals
  using Dma1Clock   = PeriphClock<AhbPeriphClockEnable,  RCC_DMA1EN,   AhbClock>;
}

#endif // ZHELE_PLATFORM_CH32_V2_CLOCK_H
