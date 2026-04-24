/**
 * @file
 * GPIO pin type aliases for NIIET K1921VG015 (Pa0..Pc15 + inverted variants).
 *
 * K1921VG015 has 3 ports (A, B, C) with 16 pins each.
 */
#pragma once

#include <zhele/common/iopin.h>
#include <zhele/ioports.h>

namespace Zhele::IO
{
  using NullPin = Zhele::IO::TPin<NullPort, 0>;

#define DECLARE_PORT_PINS(PORT_TYPE_NAME, PIN_NAME_PREFIX) \
  using PIN_NAME_PREFIX ## 0  = Zhele::IO::TPin<PORT_TYPE_NAME,  0>; \
  using PIN_NAME_PREFIX ## 1  = Zhele::IO::TPin<PORT_TYPE_NAME,  1>; \
  using PIN_NAME_PREFIX ## 2  = Zhele::IO::TPin<PORT_TYPE_NAME,  2>; \
  using PIN_NAME_PREFIX ## 3  = Zhele::IO::TPin<PORT_TYPE_NAME,  3>; \
  using PIN_NAME_PREFIX ## 4  = Zhele::IO::TPin<PORT_TYPE_NAME,  4>; \
  using PIN_NAME_PREFIX ## 5  = Zhele::IO::TPin<PORT_TYPE_NAME,  5>; \
  using PIN_NAME_PREFIX ## 6  = Zhele::IO::TPin<PORT_TYPE_NAME,  6>; \
  using PIN_NAME_PREFIX ## 7  = Zhele::IO::TPin<PORT_TYPE_NAME,  7>; \
  using PIN_NAME_PREFIX ## 8  = Zhele::IO::TPin<PORT_TYPE_NAME,  8>; \
  using PIN_NAME_PREFIX ## 9  = Zhele::IO::TPin<PORT_TYPE_NAME,  9>; \
  using PIN_NAME_PREFIX ## 10 = Zhele::IO::TPin<PORT_TYPE_NAME, 10>; \
  using PIN_NAME_PREFIX ## 11 = Zhele::IO::TPin<PORT_TYPE_NAME, 11>; \
  using PIN_NAME_PREFIX ## 12 = Zhele::IO::TPin<PORT_TYPE_NAME, 12>; \
  using PIN_NAME_PREFIX ## 13 = Zhele::IO::TPin<PORT_TYPE_NAME, 13>; \
  using PIN_NAME_PREFIX ## 14 = Zhele::IO::TPin<PORT_TYPE_NAME, 14>; \
  using PIN_NAME_PREFIX ## 15 = Zhele::IO::TPin<PORT_TYPE_NAME, 15>; \
\
  using PIN_NAME_PREFIX ## 0Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  0>; \
  using PIN_NAME_PREFIX ## 1Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  1>; \
  using PIN_NAME_PREFIX ## 2Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  2>; \
  using PIN_NAME_PREFIX ## 3Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  3>; \
  using PIN_NAME_PREFIX ## 4Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  4>; \
  using PIN_NAME_PREFIX ## 5Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  5>; \
  using PIN_NAME_PREFIX ## 6Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  6>; \
  using PIN_NAME_PREFIX ## 7Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  7>; \
  using PIN_NAME_PREFIX ## 8Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  8>; \
  using PIN_NAME_PREFIX ## 9Inv  = Zhele::IO::InvertedPin<PORT_TYPE_NAME,  9>; \
  using PIN_NAME_PREFIX ## 10Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 10>; \
  using PIN_NAME_PREFIX ## 11Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 11>; \
  using PIN_NAME_PREFIX ## 12Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 12>; \
  using PIN_NAME_PREFIX ## 13Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 13>; \
  using PIN_NAME_PREFIX ## 14Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 14>; \
  using PIN_NAME_PREFIX ## 15Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 15>;

#if defined(GPIOA)
  DECLARE_PORT_PINS(Porta, Pa)
#else
  DECLARE_PORT_PINS(NullPort, Pa)
#endif

#if defined(GPIOB)
  DECLARE_PORT_PINS(Portb, Pb)
#else
  DECLARE_PORT_PINS(NullPort, Pb)
#endif

#if defined(GPIOC)
  DECLARE_PORT_PINS(Portc, Pc)
#else
  DECLARE_PORT_PINS(NullPort, Pc)
#endif

#undef DECLARE_PORT_PINS

} // namespace Zhele::IO


