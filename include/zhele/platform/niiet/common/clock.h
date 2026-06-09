/**
 * @file
 * Clock + reset control for NIIET K1921VG015 peripherals.
 *
 * Mirrors the STM32 port's single-file-per-family layout: one generic base
 * (PeriphClock) gates a peripheral's bus clock and releases its reset, and every
 * peripheral clock (GPIO, UART, …) is a one-line specialization of it.
 *
 * NIIET differs from STM32 in two ways the base accounts for:
 *   1. enabling a peripheral needs both a clock-enable (RCU->CGCFGxxx) *and* a
 *      reset release (RCU->RSTDISxxx) — STM32 only gates the clock; and
 *   2. UART/SPI/ADC each have a dedicated reference-clock mux (UARTCLKCFG etc.)
 *      separate from the bus clock — UartClock layers that on top of the base.
 *
 * There is no clock-tree module here (no HSE/PLL/SysClock classes): the NIIET SDK
 * SystemInit() owns the tree. ClockFreq() reports the UART reference frequency,
 * the only frequency the peripheral layer needs (for the PL011 baud divisor).
 * It comes from the same source-of-truth macros the STM32 port uses — HSE_VALUE
 * (external crystal) / HSI_VALUE — or, for PLL sources, the runtime
 * SystemPll0Clock / SystemPll1Clock that SystemInit() computes.
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H
#define ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H

#include "ioreg.h"

#include <cstdint>

#ifndef HSE_VALUE
#  error "HSE_VALUE (external crystal frequency, Hz) must be defined by the build"
#endif
#ifndef HSI_VALUE
#  define HSI_VALUE 1000000u  // K1921VG015 internal oscillator is 1 MHz
#endif

extern "C" {
  // Set by the NIIET SDK SystemInit(); only read for the Pll0/Pll1 UART sources.
  extern uint32_t SystemPll0Clock;
  extern uint32_t SystemPll1Clock;
}

namespace Zhele::Clock {
  // AHB bus (GPIO lives here).
  IO_REG_WRAPPER(RCU->CGCFGAHB,  AhbClockEnable,  uint32_t);
  IO_REG_WRAPPER(RCU->RSTDISAHB, AhbResetDisable, uint32_t);
  // APB bus (UART lives here).
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

  // ---- UART (APB) ----------------------------------------------------------
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

} // namespace Zhele::Clock

#endif // ZHELE_PLATFORM_NIIET_COMMON_CLOCK_H
