/**
 * @file
 * Shared CH32 clock classes (port clocks and the HSE/HSI/PLL/SysClock tree).
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_CLOCK_H
#define ZHELE_PLATFORM_CH32_COMMON_CLOCK_H

#include "ioreg.h"

namespace Zhele::Clock
{
  // PB2PCENR on new V00x, APB2PCENR else
#if defined(CH32V00X)
  IO_REG_WRAPPER(RCC->PB2PCENR, Apb2PeriphClockEnable, uint32_t);
#else
  IO_REG_WRAPPER(RCC->APB2PCENR, Apb2PeriphClockEnable, uint32_t);
#endif

  template <typename Reg, uint32_t Mask>
  struct PortClock
  {
    static void Enable()  { Reg::Or(Mask); }
    static void Disable() { Reg::And(static_cast<uint32_t>(~Mask)); }
  };

#ifdef RCC_IOPAEN
  using PortaClock = PortClock<Apb2PeriphClockEnable, RCC_IOPAEN>;
#endif
#ifdef RCC_IOPBEN
  using PortbClock = PortClock<Apb2PeriphClockEnable, RCC_IOPBEN>;
#endif
#ifdef RCC_IOPCEN
  using PortcClock = PortClock<Apb2PeriphClockEnable, RCC_IOPCEN>;
#endif
#ifdef RCC_IOPDEN
  using PortdClock = PortClock<Apb2PeriphClockEnable, RCC_IOPDEN>;
#endif
#ifdef RCC_IOPEEN
  using PorteClock = PortClock<Apb2PeriphClockEnable, RCC_IOPEEN>;
#endif
#ifdef RCC_IOPFEN
  using PortfClock = PortClock<Apb2PeriphClockEnable, RCC_IOPFEN>;
#endif
#ifdef RCC_IOPGEN
  using PortgClock = PortClock<Apb2PeriphClockEnable, RCC_IOPGEN>;
#endif

  using ClockFrequenceT = uint32_t;

  /**
   * @brief Declare a bitfield wrapper over RCC->CFGR0 (explicit offset/length; WCH headers lack _Pos/_Msk).
   */
  #define ZHELE_CH32_DECLARE_CFGR0_BITFIELD(CLASS_NAME, FIELD_OFFSET, FIELD_LENGTH) \
    static constexpr unsigned CLASS_NAME##Offset = (FIELD_OFFSET); \
    struct CLASS_NAME \
    { \
      static constexpr unsigned Offset = (FIELD_OFFSET); \
      static constexpr uint32_t Mask = ((uint32_t(1u) << (FIELD_LENGTH)) - 1); \
      static uint32_t Get() { return (RCC->CFGR0 >> Offset) & Mask; } \
      static void Set(uint32_t value) \
      { \
        RCC->CFGR0 = (RCC->CFGR0 & ~(Mask << Offset)) | ((value & Mask) << Offset); \
      } \
    }

  /**
   * @brief Base class for clock sources (HSE / HSI / PLL).
   */
  class ClockBase
  {
  protected:
    static constexpr uint32_t ClockStartTimeout = 4000;

    /// Enable clock source and wait for ready.
    static bool EnableClockSource(uint32_t turnMask, uint32_t waitReadyMask)
    {
      uint32_t timeout = ClockStartTimeout;
      RCC->CTLR |= turnMask;
      while (((RCC->CTLR & waitReadyMask) == 0) && --timeout)
        ;
      return timeout != 0;
    }

    /// Disable clock source and wait for ready to drop.
    static bool DisableClockSource(uint32_t turnMask, uint32_t waitReadyMask)
    {
      uint32_t timeout = ClockStartTimeout;
      RCC->CTLR &= ~turnMask;
      while (((RCC->CTLR & waitReadyMask) != 0) && --timeout)
        ;
      return timeout != 0;
    }
  };

  /**
   * @brief External high-speed oscillator (HSE).
   */
  class HseClock : public ClockBase
  {
  public:
    static constexpr ClockFrequenceT SrcClockFreq() { return HSE_VALUE; }
    static constexpr unsigned GetDivider() { return 1; }
    static constexpr unsigned GetMultipler() { return 1; }
    static constexpr ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

    static bool Enable()  { return ClockBase::EnableClockSource(RCC_HSEON, RCC_HSERDY); }
    static bool Disable() { return ClockBase::DisableClockSource(RCC_HSEON, RCC_HSERDY); }
  };

  /**
   * @brief Internal high-speed oscillator (HSI).
   */
  class HsiClock : public ClockBase
  {
  public:
    static constexpr ClockFrequenceT SrcClockFreq() { return HSI_VALUE; }
    static constexpr unsigned GetDivider() { return 1; }
    static constexpr unsigned GetMultipler() { return 1; }
    static constexpr ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

    static bool Enable()  { return ClockBase::EnableClockSource(RCC_HSION, RCC_HSIRDY); }
    static bool Disable() { return ClockBase::DisableClockSource(RCC_HSION, RCC_HSIRDY); }
  };

  /**
   * @brief PLL clock source (method bodies live in the family header).
   */
  class PllClock : public ClockBase
  {
  public:
    /// PLL input selection.
    enum ClockSource
    {
      Internal,   ///< HSI drives the PLL
      External    ///< HSE drives the PLL
    };

    static ClockFrequenceT SrcClockFreq();

    static unsigned GetDivider();
    static unsigned GetMultipler();

    template<unsigned multiplier>
    static void SetMultiplier();

    template<ClockSource clockSource>
    static void SelectClockSource();
    static ClockSource GetClockSource();

    static ClockFrequenceT ClockFreq();

    static bool Enable()  { return ClockBase::EnableClockSource(RCC_PLLON, RCC_PLLRDY); }
    static bool Disable() { return ClockBase::DisableClockSource(RCC_PLLON, RCC_PLLRDY); }
  };

  /**
   * @brief System clock (SYSCLK) — selects HSI / HSE / PLL.
   */
  class SysClock
  {
  public:
    /// System clock source.
    enum ClockSource
    {
      Internal,   ///< HSI
      External,   ///< HSE
      Pll         ///< PLL
    };

    /// Result codes for SelectClockSource().
    enum ErrorCode
    {
      Success,
      ClockSourceFailed,
      InvalidClockSource,
      ClockSelectFailed
    };

    // CFGR0.SWS readback values.
    static constexpr uint32_t SwsHsi = 0x00u;
    static constexpr uint32_t SwsHse = 0x04u;
    static constexpr uint32_t SwsPll = 0x08u;

    /// Max SYSCLK for this family (defined per family).
    static ClockFrequenceT MaxFreq();

    /// Enable @p clockSource and switch SYSCLK to it.
    template<ClockSource clockSource>
    static ErrorCode SelectClockSource();

    /// Current SYSCLK frequency.
    static ClockFrequenceT ClockFreq();

    static ClockFrequenceT SrcClockFreq() { return ClockFreq(); }
  };

  template<SysClock::ClockSource clockSource>
  inline SysClock::ErrorCode SysClock::SelectClockSource()
  {
    uint32_t swBits;
    uint32_t swsValue;

    if constexpr (clockSource == Internal)
    {
      if (!HsiClock::Enable())
        return ClockSourceFailed;
      swBits = RCC_SW_HSI;
      swsValue = SwsHsi;
    }
    else if constexpr (clockSource == External)
    {
      if (!HseClock::Enable())
        return ClockSourceFailed;
      swBits = RCC_SW_HSE;
      swsValue = SwsHse;
    }
    else if constexpr (clockSource == Pll)
    {
      if (!PllClock::Enable())
        return ClockSourceFailed;
      swBits = RCC_SW_PLL;
      swsValue = SwsPll;
    }
    else
    {
      return InvalidClockSource;
    }

    RCC->CFGR0 = (RCC->CFGR0 & ~static_cast<uint32_t>(RCC_SW)) | swBits;

    uint32_t timeout = 10000;
    while (((RCC->CFGR0 & RCC_SWS) != swsValue) && --timeout)
      ;

    return timeout != 0 ? Success : ClockSelectFailed;
  }

  inline ClockFrequenceT SysClock::ClockFreq()
  {
    switch (RCC->CFGR0 & RCC_SWS)
    {
      case SwsHse: return HseClock::ClockFreq();
      case SwsPll: return PllClock::ClockFreq();
      case SwsHsi:
      default:     return HsiClock::ClockFreq();
    }
  }

  /**
   * @brief Base class for a bus clock (AHB / APB).
   */
  template<typename _SrcClock, typename _PrescalerBitField>
  class BusClock
  {
  public:
    static ClockFrequenceT SrcClockFreq() { return _SrcClock::ClockFreq(); }
    static ClockFrequenceT ClockFreq() { return SrcClockFreq(); }

    template<typename PrescalerType>
    static void SetPrescaler(PrescalerType prescaler)
    {
      _PrescalerBitField::Set(static_cast<uint32_t>(prescaler));
    }
  };
}

#endif // ZHELE_PLATFORM_CH32_COMMON_CLOCK_H
