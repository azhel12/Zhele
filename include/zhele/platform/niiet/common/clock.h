/**
 * @file
 * Clock + reset control for NIIET peripherals (K1921VG015, K1921VG5T, K1921VG7T).
 *
 * Mirrors the STM32 port's single-file-per-family layout: one generic base
 * (PeriphClock) gates a peripheral's bus clock and releases its reset, and every
 * peripheral clock (GPIO, UART, SPI, …) is a one-line specialization of it.
 *
 * NIIET differs from STM32 in two ways the base accounts for:
 *   1. enabling a peripheral needs both a clock-enable (RCU->CGCFGxxx) *and* a
 *      reset release (RCU->RSTDISxxx) — STM32 only gates the clock; and
 *   2. UART/SPI/ADC each have a dedicated reference-clock mux (UARTCLKCFG on
 *      K1921VG015, UARTCFG/SPICFG on VG5T/VG7T) separate from the bus clock —
 *      UartClock / SpiClock layer that on top of the base.
 *
 * There is no clock-tree module here (no HSE/PLL/SysClock classes): the NIIET SDK
 * SystemInit() owns the tree. ClockFreq() reports the reference frequency of a
 * peripheral, the only frequency the peripheral layer needs (PL011 baud divisor,
 * PL022 SCK divisor). It comes from the same source-of-truth
 * macros the STM32 port uses — HSE_VALUE (external crystal) / HSI_VALUE — or, for
 * PLL / bus sources, the runtime globals SystemInit() computes.
 *
 * The two device generations share the bus registers (CGCFGAHB/APB, RSTDISAHB/APB)
 * and therefore the whole GPIO section; they differ in the per-peripheral clock
 * muxes, so those sit in device-guarded sections below.
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H
#define ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H

#include "ioreg.h"

#include <cstdint>

#ifndef HSE_VALUE
#  error "HSE_VALUE (external crystal frequency, Hz) must be defined by the build"
#endif
#ifndef HSI_VALUE
#  if defined(ZHELE_NIIET_SERIES_VGXT)
#    define HSI_VALUE 4000000u  // K1921VG5T/VG7T internal RC oscillator is 4 MHz
#  else
#    define HSI_VALUE 1000000u  // K1921VG015 internal oscillator is 1 MHz
#  endif
#endif

extern "C" {
#if defined(ZHELE_NIIET_SERIES_VGXT)
  // Set by the NIIET SDK SystemInit(): the single system PLL and the resulting
  // core/bus clock. SystemCoreClockUpdate() recomputes both from the RCU tree.
  extern uint32_t SystemPllClock;
  extern uint32_t SystemCoreClock;
  void SystemCoreClockUpdate(void);
#else
  // K1921VG015 has two PLL outputs; only read for the Pll0/Pll1 UART sources.
  extern uint32_t SystemPll0Clock;
  extern uint32_t SystemPll1Clock;
  extern uint32_t SystemCoreClock;
  void SystemCoreClockUpdate(void);
#endif
}

namespace Zhele::Clock {
  // AHB bus (GPIO lives here).
  IO_REG_WRAPPER(RCU->CGCFGAHB,  AhbClockEnable,  uint32_t);
  IO_REG_WRAPPER(RCU->RSTDISAHB, AhbResetDisable, uint32_t);
  // APB bus (UART, SPI, timers).
  IO_REG_WRAPPER(RCU->CGCFGAPB,  ApbClockEnable,  uint32_t);
  IO_REG_WRAPPER(RCU->RSTDISAPB, ApbResetDisable, uint32_t);

  /**
   * @brief Generic clock-gate + reset-release for one peripheral on one bus.
   *
   * The clock-enable and reset-disable registers share bit positions per
   * peripheral, so a single mask drives both.
   *
   * @tparam _ClkEn   Clock-enable register wrapper (CGCFGxxx).
   * @tparam _RstDis  Reset-disable register wrapper (RSTDISxxx).
   * @tparam _Mask    Peripheral bit in both registers.
   */
  template <typename _ClkEn, typename _RstDis, uint32_t _Mask>
  struct PeriphClock {
    static void Enable() {
      _ClkEn::Or(_Mask);
      _RstDis::Or(_Mask);
    }
    static void Disable() {
      _RstDis::And(static_cast<uint32_t>(~_Mask));
      _ClkEn::And(static_cast<uint32_t>(~_Mask));
    }
  };

  /// Live core / bus clock (AHB = APB = SYSCLK — there are no bus prescalers).
  inline uint32_t BusClockFreq() {
    SystemCoreClockUpdate();
    return SystemCoreClock;
  }

  // ---- GPIO ports (AHB) ----------------------------------------------------
  template <uint32_t _Mask>
  using PortClock = PeriphClock<AhbClockEnable, AhbResetDisable, _Mask>;

#ifdef RCU_CGCFGAHB_GPIOAEN_Msk
  using PortaClock = PortClock<RCU_CGCFGAHB_GPIOAEN_Msk>;
#endif
#ifdef RCU_CGCFGAHB_GPIOBEN_Msk
  using PortbClock = PortClock<RCU_CGCFGAHB_GPIOBEN_Msk>;
#endif
#ifdef RCU_CGCFGAHB_GPIOCEN_Msk
  using PortcClock = PortClock<RCU_CGCFGAHB_GPIOCEN_Msk>;
#endif

#if defined(ZHELE_NIIET_SERIES_VGXT)
  // =========================== K1921VG5T / K1921VG7T =========================
  // Peripheral reference clocks are configured through one register shape shared
  // by UARTCFG[n] / SPICFG[n] / ADCCFG / WDTCFG: CLKEN, RSTDIS, CLKSEL (source
  // mux) and an optional divider by 2*(DIVN+1) enabled with DIVEN.

  // EXTCLK is a board property like HSE (clock fed to an EXTCLK pin); the NIIET
  // SDK assumes 1 MHz when the project does not override it.
#ifndef EXTCLK_VALUE
#  define EXTCLK_VALUE 1000000u
#endif

  /// Peripheral reference clock source — values match RCU->{UART,SPI}CFG.CLKSEL.
  enum class PeriphClockSource : uint32_t {
    Hsi  = 0,  ///< REFCLK: HSI 4 MHz (when the platform CPE bit is 0)
    Hse  = 1,  ///< SRCCLK: HSE — external crystal (when CPE = 0)
    Pll  = 2,  ///< SYSPLLCLK
    Ext  = 3,  ///< EXTCLK — clock taken from a GPIO pin
  };

  /**
   * @brief Clock control for one peripheral behind a *CFG reference-clock mux.
   *
   * @tparam _Cfg      Accessor type with `static volatile uint32_t& Get()` for
   *                   the RCU *CFG register of this instance.
   * @tparam _ApbMask  CGCFGAPB / RSTDISAPB enable bit for this peripheral.
   * @tparam _Src      Reference clock source.
   * @tparam _Div      Extra division: 0 = none, otherwise the clock is divided
   *                   by 2*(_Div+1) (hardware DIVEN + DIVN fields, _Div ≤ 63).
   *
   * The RCU_UARTCFG_* bit positions are used for every instance on purpose:
   * UARTCFG, SPICFG, ADCCFG and WDTCFG share the identical field layout, and the
   * SDK header only spells the positions out once per register name.
   */
  template <typename _Cfg, uint32_t _ApbMask, PeriphClockSource _Src, uint32_t _Div>
  struct RefClock : PeriphClock<ApbClockEnable, ApbResetDisable, _ApbMask> {
    static_assert(_Div <= 63, "RCU *CFG.DIVN is 6 bits wide");

    using Base = PeriphClock<ApbClockEnable, ApbResetDisable, _ApbMask>;

    static void Enable() {
      Base::Enable();  // APB clock + reset release
      uint32_t cfg = (static_cast<uint32_t>(_Src) << RCU_UARTCFG_CLKSEL_Pos)
                   | RCU_UARTCFG_CLKEN_Msk
                   | RCU_UARTCFG_RSTDIS_Msk;
      if constexpr (_Div != 0)
        cfg |= RCU_UARTCFG_DIVEN_Msk | (_Div << RCU_UARTCFG_DIVN_Pos);
      _Cfg::Get() = cfg;
    }

    static void Disable() {
      _Cfg::Get() = 0;
      Base::Disable();
    }

    /// Reference clock in Hz, derived from the selected source and divider.
    static uint32_t ClockFreq() {
      uint32_t source;
      if constexpr (_Src == PeriphClockSource::Hsi)
        source = HSI_VALUE;
      else if constexpr (_Src == PeriphClockSource::Hse)
        source = HSE_VALUE;
      else if constexpr (_Src == PeriphClockSource::Pll)
        source = (SystemCoreClockUpdate(), SystemPllClock);
      else
        source = EXTCLK_VALUE;

      if constexpr (_Div != 0)
        return source / (2u * (_Div + 1u));
      else
        return source;
    }
  };

  // ---- UART (APB + UARTCFG[n]) ---------------------------------------------
  /**
   * Default source is HSE: it matches the NIIET SDK retarget, is independent of
   * whether the PLL is configured, and is accurate enough for high baud rates.
   */
  #define ZHELE_NIIET_UART_CFG(n) \
    struct UartCfg##n { static volatile uint32_t& Get() { return RCU->UARTCFG[n].UARTCFG; } };

  ZHELE_NIIET_UART_CFG(0)
  ZHELE_NIIET_UART_CFG(1)
  #undef ZHELE_NIIET_UART_CFG

  template <typename _Cfg, uint32_t _ApbMask,
            PeriphClockSource _Src = PeriphClockSource::Hse, uint32_t _Div = 0>
  using UartClockT = RefClock<_Cfg, _ApbMask, _Src, _Div>;

#ifdef RCU_CGCFGAPB_UART0EN_Msk
  using Uart0Clock = UartClockT<UartCfg0, RCU_CGCFGAPB_UART0EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_UART1EN_Msk
  using Uart1Clock = UartClockT<UartCfg1, RCU_CGCFGAPB_UART1EN_Msk>;
#endif

  // ---- SPI (APB + SPICFG[n]) -----------------------------------------------
  #define ZHELE_NIIET_SPI_CFG(n) \
    struct SpiCfg##n { static volatile uint32_t& Get() { return RCU->SPICFG[n].SPICFG; } };

  ZHELE_NIIET_SPI_CFG(0)
  ZHELE_NIIET_SPI_CFG(1)
#ifdef SPI2
  ZHELE_NIIET_SPI_CFG(2)
#endif
  #undef ZHELE_NIIET_SPI_CFG

  /**
   * SPI defaults to the PLL: the PL022 SCK divisor chain (CPSDVSR × (1 + SCR))
   * only divides, so a fast reference is what makes fast SCK reachable.
   */
  template <typename _Cfg, uint32_t _ApbMask,
            PeriphClockSource _Src = PeriphClockSource::Pll, uint32_t _Div = 0>
  using SpiClockT = RefClock<_Cfg, _ApbMask, _Src, _Div>;

#ifdef RCU_CGCFGAPB_SPI0EN_Msk
  using Spi0Clock = SpiClockT<SpiCfg0, RCU_CGCFGAPB_SPI0EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_SPI1EN_Msk
  using Spi1Clock = SpiClockT<SpiCfg1, RCU_CGCFGAPB_SPI1EN_Msk>;
#endif
#if defined(RCU_CGCFGAPB_SPI2EN_Msk) && defined(SPI2)
  using Spi2Clock = SpiClockT<SpiCfg2, RCU_CGCFGAPB_SPI2EN_Msk>;
#endif

  // ---- DMA (APB on VG5T, always clocked on VG7T) ---------------------------
#ifdef RCU_CGCFGAPB_DMAEN_Msk
  using DmaClock = PeriphClock<ApbClockEnable, ApbResetDisable, RCU_CGCFGAPB_DMAEN_Msk>;
#else
  /// K1921VG7T has no DMA gate bit — the controller is always clocked.
  struct DmaClock {
    static void Enable() {}
    static void Disable() {}
  };
#endif

  // ---- Timers (APB, bus-clocked) -------------------------------------------
  template <uint32_t _ApbMask>
  struct TimerClockT : PeriphClock<ApbClockEnable, ApbResetDisable, _ApbMask> {
    static uint32_t ClockFreq() { return BusClockFreq(); }
  };

#ifdef RCU_CGCFGAPB_TMR0EN_Msk
  using Timer0Clock = TimerClockT<RCU_CGCFGAPB_TMR0EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_TMR1EN_Msk
  using Timer1Clock = TimerClockT<RCU_CGCFGAPB_TMR1EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_TMR2EN_Msk
  using Timer2Clock = TimerClockT<RCU_CGCFGAPB_TMR2EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_TMR3EN_Msk
  using Timer3Clock = TimerClockT<RCU_CGCFGAPB_TMR3EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_TMR4EN_Msk
  using Timer4Clock = TimerClockT<RCU_CGCFGAPB_TMR4EN_Msk>;
#endif

#else
  // ================================ K1921VG015 ==============================

  /// UART reference clock source — values match RCU->UARTCLKCFG.CLKSEL.
  enum class UartClockSource : uint32_t {
    Hsi  = 0,  ///< HSI 1 MHz (when CPE = 0)
    Hse  = 1,  ///< HSE — external crystal (when CPE = 0)
    Pll0 = 2,  ///< SYSPLL0CLK
    Pll1 = 3,  ///< SYSPLL1CLK
  };

  /**
   * @brief Clock control for a single UART instance: bus gate/reset (via the
   *        PeriphClock base) plus the per-instance reference-clock mux.
   *
   * Default source is HSE: it matches the NIIET SDK retarget, is independent of
   * whether the PLL is configured, and is the only source accurate enough for
   * high baud rates (HSI is 1 MHz — too slow for 115200). With the platform CPE
   * bit set the silicon swaps the HSI/HSE meaning of CLKSEL (see K1921VG015.h);
   * this board runs CPE = 0 (crystal → HSE).
   *
   * @tparam _Number   UART index (0..4) — selects RCU->UARTCLKCFG[_Number].
   * @tparam _ApbMask  CGCFGAPB / RSTDISAPB enable bit for this UART.
   * @tparam _Src      Reference clock source (default HSE).
   */
  template <unsigned _Number, uint32_t _ApbMask, UartClockSource _Src = UartClockSource::Hse>
  struct UartClock : PeriphClock<ApbClockEnable, ApbResetDisable, _ApbMask> {
    using Base = PeriphClock<ApbClockEnable, ApbResetDisable, _ApbMask>;

    static void Enable() {
      Base::Enable();  // APB clock + reset release
      // Select the reference clock source and release the UART clock from reset.
      RCU->UARTCLKCFG[_Number].UARTCLKCFG =
          (static_cast<uint32_t>(_Src) << RCU_UARTCLKCFG_CLKSEL_Pos)
          | RCU_UARTCLKCFG_CLKEN_Msk | RCU_UARTCLKCFG_RSTDIS_Msk;
    }

    static void Disable() {
      RCU->UARTCLKCFG[_Number].UARTCLKCFG = 0;
      Base::Disable();
    }

    /// UART reference clock in Hz, derived from the selected source (see above).
    static uint32_t ClockFreq() {
      if constexpr (_Src == UartClockSource::Hsi)
        return HSI_VALUE;
      else if constexpr (_Src == UartClockSource::Hse)
        return HSE_VALUE;
      else if constexpr (_Src == UartClockSource::Pll0)
        return SystemPll0Clock;
      else
        return SystemPll1Clock;
    }
  };

#ifdef RCU_CGCFGAPB_UART0EN_Msk
  using Uart0Clock = UartClock<0, RCU_CGCFGAPB_UART0EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_UART1EN_Msk
  using Uart1Clock = UartClock<1, RCU_CGCFGAPB_UART1EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_UART2EN_Msk
  using Uart2Clock = UartClock<2, RCU_CGCFGAPB_UART2EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_UART3EN_Msk
  using Uart3Clock = UartClock<3, RCU_CGCFGAPB_UART3EN_Msk>;
#endif
#ifdef RCU_CGCFGAPB_UART4EN_Msk
  using Uart4Clock = UartClock<4, RCU_CGCFGAPB_UART4EN_Msk>;
#endif

#endif // ZHELE_NIIET_SERIES_VGXT

} // namespace Zhele::Clock

#endif // ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H
