/**
 * @file
 * CH32V00x: instantiate GPIO ports (device + clocks + shared PortImplementation).
 */
#pragma once

#include <ch32v00x.h>

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

#if defined(GPIOA)
  MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
#endif

#if defined(GPIOB)
  MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
#endif

#if defined(GPIOC)
  MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')
#endif

#if defined(GPIOD)
  MAKE_PORT(GPIOD, Zhele::Clock::PortdClock, Portd, 'D')
#endif

#if defined(GPIOE)
  MAKE_PORT(GPIOE, Zhele::Clock::PorteClock, Porte, 'E')
#endif

#if defined(GPIOF)
  MAKE_PORT(GPIOF, Zhele::Clock::PortfClock, Portf, 'F')
#endif

#if defined(GPIOG)
  MAKE_PORT(GPIOG, Zhele::Clock::PortgClock, Portg, 'G')
#endif

#undef MAKE_PORT
}


