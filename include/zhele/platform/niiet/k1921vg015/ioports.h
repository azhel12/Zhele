/**
 * @file
 * K1921VG015: instantiate GPIO port types (Porta, Portb, Portc).
 */
#pragma once

#include <K1921VG015.h>

#include "../common/clock_gpio_ahb.h"
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

#if defined(GPIOA)
  MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
#endif

#if defined(GPIOB)
  MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
#endif

#if defined(GPIOC)
  MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')
#endif

#undef MAKE_PORT

} // namespace Zhele::IO


