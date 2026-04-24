/**
 * @file
 * Clock enable via RCC_APB2PCENR (WCH CH32: V00x / V10x / V20x / V30x share RCC_IOPxEN + APB2PCENR).
 */
#pragma once

#include "ioreg.h"

namespace Zhele::Clock {
  IO_REG_WRAPPER(RCC->APB2PCENR, Apb2PeriphClockEnable, uint32_t);

  template <typename Reg, uint32_t Mask>
  struct PortClock {
    static void Enable() { Reg::Or(Mask); }
    static void Disable() { Reg::And(static_cast<uint32_t>(~Mask)); }
  };

  using PortaClock = PortClock<Apb2PeriphClockEnable, RCC_IOPAEN>;
  using PortbClock = PortClock<Apb2PeriphClockEnable, RCC_IOPBEN>;
  using PortcClock = PortClock<Apb2PeriphClockEnable, RCC_IOPCEN>;
  using PortdClock = PortClock<Apb2PeriphClockEnable, RCC_IOPDEN>;
}


