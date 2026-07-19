/**
 * @file
 * CH32V00x clocks: GPIO via RCC->APB2PCENR, plus peripheral and bus clocks.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_CLOCK_H
#define ZHELE_PLATFORM_CH32_V0_CLOCK_H

#include <ch32v00x.h>
#include "../common/clock.h"

namespace Zhele::Clock
{
  // PB1PCENR / HBPCENR on new V00x, APB1PCENR / AHBPCENR else
#if defined(CH32V00X)
  IO_REG_WRAPPER(RCC->PB1PCENR, Apb1PeriphClockEnable, uint32_t);
  IO_REG_WRAPPER(RCC->HBPCENR,  AhbPeriphClockEnable,  uint32_t);
#else
  IO_REG_WRAPPER(RCC->APB1PCENR, Apb1PeriphClockEnable, uint32_t);
  IO_REG_WRAPPER(RCC->AHBPCENR,  AhbPeriphClockEnable,  uint32_t);
#endif

  inline ClockFrequenceT PllClock::SrcClockFreq()
  {
    return GetClockSource() == External ? HSE_VALUE : HSI_VALUE;
  }

  inline unsigned PllClock::GetDivider() { return 1; }

  inline unsigned PllClock::GetMultipler() { return 2; } // fixed x2 on V003/V00x

  template<unsigned multiplier>
  inline void PllClock::SetMultiplier()
  {
    static_assert(multiplier == 2, "CH32V00x PLL multiplier is fixed at x2");
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
   * @brief AHB (HCLK) clock — supports the V00x odd dividers (/3, /5, /6, /7).
   */
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
      // indices 0..7 divide by (n+1); 8..15 shift right
      static constexpr uint8_t clockPresc[16] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
      uint32_t index = AhbPrescalerBitField::Get();
      ClockFrequenceT clock = SysClock::ClockFreq();
      return index < 8 ? (clock / clockPresc[index]) : (clock >> clockPresc[index]);
    }

    template<Prescaler prescaler>
    static void SetPrescaler()
    {
      Base::SetPrescaler(prescaler);
    }
  };

  /**
   * @brief APB1 (PCLK1) clock — V00x has no APB prescaler, so PCLK1 = HCLK.
   */
  class Apb1Clock
  {
  public:
    static ClockFrequenceT SrcClockFreq() { return AhbClock::ClockFreq(); }
    static ClockFrequenceT ClockFreq()    { return AhbClock::ClockFreq(); }
  };

  /// APB2 (PCLK2) clock — identical to APB1/HCLK on V00x.
  using Apb2Clock = Apb1Clock;

  inline ClockFrequenceT SysClock::MaxFreq() { return 48000000u; }

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

  using I2c1Clock = PeriphClock<Apb1PeriphClockEnable, RCC_I2C1EN, Apb1Clock>;
  using Dma1Clock = PeriphClock<AhbPeriphClockEnable,  RCC_DMA1EN, AhbClock>;

  using Usart1Clock = PeriphClock<Apb2PeriphClockEnable, RCC_USART1EN, Apb2Clock>;
#if defined(RCC_USART2EN)
  using Usart2Clock = PeriphClock<Apb2PeriphClockEnable, RCC_USART2EN, Apb2Clock>;
#endif

  // TIM1 on APB2, TIM2/TIM3 on APB1 (TIM3 only on V00x, not V003)
  using Tim1Clock = PeriphClock<Apb2PeriphClockEnable, RCC_TIM1EN, Apb2Clock>;
  using Tim2Clock = PeriphClock<Apb1PeriphClockEnable, RCC_TIM2EN, Apb1Clock>;
#if defined(RCC_TIM3EN)
  using Tim3Clock = PeriphClock<Apb1PeriphClockEnable, RCC_TIM3EN, Apb1Clock>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V0_CLOCK_H
