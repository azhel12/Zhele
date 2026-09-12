/**
 * @file
 * K1921VG7T: instantiate GPIO port types (Porta, Portb).
 *
 * Port A is 16-bit, port B is 15-bit (B15 does not exist on the LQFP-48 package).
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG7T_IOPORTS_H
#define ZHELE_PLATFORM_NIIET_K1921VG7T_IOPORTS_H

#include <K1921VG7T.h>

#include "../common/clock.h"
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

#undef MAKE_PORT

} // namespace Zhele::IO

#endif // ZHELE_PLATFORM_NIIET_K1921VG7T_IOPORTS_H
