/**
 * @file
 * GPIO clock and reset control via RCU->CGCFGAHB / RCU->RSTDISAHB for NIIET K1921VG015.
 *
 * Enable()  — gates on the AHB clock and releases the peripheral from reset.
 * Disable() — puts the peripheral back in reset and gates off the clock.
 */
#pragma once

#include "ioreg.h"

namespace Zhele::Clock {
  IO_REG_WRAPPER(RCU->CGCFGAHB,  AhbClockEnable,  uint32_t);
  IO_REG_WRAPPER(RCU->RSTDISAHB, AhbResetDisable, uint32_t);

  // Both CGCFGAHB and RSTDISAHB use the same bit positions for GPIO (bits 8-10).
  template <uint32_t Mask>
  struct PortClock {
    static void Enable() {
      AhbClockEnable::Or(Mask);
      AhbResetDisable::Or(Mask);
    }
    static void Disable() {
      AhbResetDisable::And(static_cast<uint32_t>(~Mask));
      AhbClockEnable::And(static_cast<uint32_t>(~Mask));
    }
  };

#ifdef RCU_CGCFGAHB_GPIOAEN_Msk
  using PortaClock = PortClock<RCU_CGCFGAHB_GPIOAEN_Msk>;
#endif
#ifdef RCU_CGCFGAHB_GPIOBEN_Msk
  using PortbClock = PortClock<RCU_CGCFGAHB_GPIOBEN_Msk>;
#endif
#ifdef RCU_CGCFGAHB_GPIOCEN_Msk
  using PortcClock = PortClock<RCU_CGCFGAHB_GPIOCEN_Msk>;
#endif

} // namespace Zhele::Clock


