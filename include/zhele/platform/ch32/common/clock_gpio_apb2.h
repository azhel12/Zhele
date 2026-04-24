/**
 * @file
 * GPIO clock enable via RCC->APB2PCENR for CH32 V00x / V10x / V20x / V30x.
 */
#pragma once

#include "ioreg.h"

namespace Zhele::Clock
{
  //  PB2PCENR for V00x (expcept V003), APB2PCENR else
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
}


