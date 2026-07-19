/**
 * @file
 * CH32V20x: instantiate GPIO ports (device + clocks + shared PortImplementation).
 *
 * @note CH32V203 (D6 variant) physically exposes ports A-D. ch32v20x.h also
 *       #defines GPIOE/F/G, but provides no RCC_IOP{E,F,G}EN clock-enable bit,
 *       so those ports are only instantiated when a clock bit actually exists.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_IOPORTS_H
#define ZHELE_PLATFORM_CH32_V2_IOPORTS_H

#include <ch32v20x.h>

#include "clock.h"

#include "../common/ioreg.h"
#include "../common/ioports_gpio.h"

namespace Zhele::IO
{

#define MAKE_PORT(REGS, ClkEnReg, className, ID) \
  namespace Private \
  { \
    IO_STRUCT_WRAPPER(REGS, className##Regs, GPIO_TypeDef); \
  } \
  using className = Private::PortImplementation<Private::className##Regs, ClkEnReg, ID>;

#if defined(GPIOA) && defined(RCC_IOPAEN)
  MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
#endif

#if defined(GPIOB) && defined(RCC_IOPBEN)
  MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
#endif

#if defined(GPIOC) && defined(RCC_IOPCEN)
  MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')
#endif

#if defined(GPIOD) && defined(RCC_IOPDEN)
  MAKE_PORT(GPIOD, Zhele::Clock::PortdClock, Portd, 'D')
#endif

#if defined(GPIOE) && defined(RCC_IOPEEN)
  MAKE_PORT(GPIOE, Zhele::Clock::PorteClock, Porte, 'E')
#endif

#if defined(GPIOF) && defined(RCC_IOPFEN)
  MAKE_PORT(GPIOF, Zhele::Clock::PortfClock, Portf, 'F')
#endif

#if defined(GPIOG) && defined(RCC_IOPGEN)
  MAKE_PORT(GPIOG, Zhele::Clock::PortgClock, Portg, 'G')
#endif

#undef MAKE_PORT
}

#endif // ZHELE_PLATFORM_CH32_V2_IOPORTS_H
