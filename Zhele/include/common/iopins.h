/**
 * @file
 * Contains instances of gpio pins (as typedefs)
 * 
 * @author Konstantin Chizhov
 * @date 2010
 * @license FreeBSD
 */

#ifndef ZHELE_IOPINS_COMMON_H
#define ZHELE_IOPINS_COMMON_H

#include "iopin.h"

#include <ioports.h>

namespace Zhele::IO
{
	using NullPin = Zhele::IO::TPin<NullPort, 0>;
#define DECLARE_PORT_PINS(PORT_TYPE_NAME, PIN_NAME_PREFIX) \
	using PIN_NAME_PREFIX ## 0 = Zhele::IO::TPin<PORT_TYPE_NAME, 0>; \
	using PIN_NAME_PREFIX ## 1 = Zhele::IO::TPin<PORT_TYPE_NAME, 1>; \
	using PIN_NAME_PREFIX ## 2 = Zhele::IO::TPin<PORT_TYPE_NAME, 2>; \
	using PIN_NAME_PREFIX ## 3 = Zhele::IO::TPin<PORT_TYPE_NAME, 3>; \
	using PIN_NAME_PREFIX ## 4 = Zhele::IO::TPin<PORT_TYPE_NAME, 4>; \
	using PIN_NAME_PREFIX ## 5 = Zhele::IO::TPin<PORT_TYPE_NAME, 5>; \
	using PIN_NAME_PREFIX ## 6 = Zhele::IO::TPin<PORT_TYPE_NAME, 6>; \
	using PIN_NAME_PREFIX ## 7 = Zhele::IO::TPin<PORT_TYPE_NAME, 7>; \
	using PIN_NAME_PREFIX ## 8 = Zhele::IO::TPin<PORT_TYPE_NAME, 8>; \
	using PIN_NAME_PREFIX ## 9 = Zhele::IO::TPin<PORT_TYPE_NAME, 9>; \
	using PIN_NAME_PREFIX ## 10 = Zhele::IO::TPin<PORT_TYPE_NAME, 10>; \
	using PIN_NAME_PREFIX ## 11 = Zhele::IO::TPin<PORT_TYPE_NAME, 11>; \
	using PIN_NAME_PREFIX ## 12 = Zhele::IO::TPin<PORT_TYPE_NAME, 12>; \
	using PIN_NAME_PREFIX ## 13 = Zhele::IO::TPin<PORT_TYPE_NAME, 13>; \
	using PIN_NAME_PREFIX ## 14 = Zhele::IO::TPin<PORT_TYPE_NAME, 14>; \
	using PIN_NAME_PREFIX ## 15 = Zhele::IO::TPin<PORT_TYPE_NAME, 15>; \
	using PIN_NAME_PREFIX ## 16 = Zhele::IO::TPin<PORT_TYPE_NAME, 16>; \
	using PIN_NAME_PREFIX ## 17 = Zhele::IO::TPin<PORT_TYPE_NAME, 17>; \
	using PIN_NAME_PREFIX ## 18 = Zhele::IO::TPin<PORT_TYPE_NAME, 18>; \
	using PIN_NAME_PREFIX ## 19 = Zhele::IO::TPin<PORT_TYPE_NAME, 19>; \
	using PIN_NAME_PREFIX ## 20 = Zhele::IO::TPin<PORT_TYPE_NAME, 20>; \
	using PIN_NAME_PREFIX ## 21 = Zhele::IO::TPin<PORT_TYPE_NAME, 21>; \
	using PIN_NAME_PREFIX ## 22 = Zhele::IO::TPin<PORT_TYPE_NAME, 22>; \
	using PIN_NAME_PREFIX ## 23 = Zhele::IO::TPin<PORT_TYPE_NAME, 23>; \
	using PIN_NAME_PREFIX ## 24 = Zhele::IO::TPin<PORT_TYPE_NAME, 24>; \
	using PIN_NAME_PREFIX ## 25 = Zhele::IO::TPin<PORT_TYPE_NAME, 25>; \
	using PIN_NAME_PREFIX ## 26 = Zhele::IO::TPin<PORT_TYPE_NAME, 26>; \
	using PIN_NAME_PREFIX ## 27 = Zhele::IO::TPin<PORT_TYPE_NAME, 27>; \
	using PIN_NAME_PREFIX ## 28 = Zhele::IO::TPin<PORT_TYPE_NAME, 28>; \
	using PIN_NAME_PREFIX ## 29 = Zhele::IO::TPin<PORT_TYPE_NAME, 29>; \
	using PIN_NAME_PREFIX ## 30 = Zhele::IO::TPin<PORT_TYPE_NAME, 30>; \
	using PIN_NAME_PREFIX ## 31 = Zhele::IO::TPin<PORT_TYPE_NAME, 31>; \
\
	using PIN_NAME_PREFIX ## 0 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 0>; \
	using PIN_NAME_PREFIX ## 1 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 1>; \
	using PIN_NAME_PREFIX ## 2 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 2>; \
	using PIN_NAME_PREFIX ## 3 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 3>; \
	using PIN_NAME_PREFIX ## 4 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 4>; \
	using PIN_NAME_PREFIX ## 5 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 5>; \
	using PIN_NAME_PREFIX ## 6 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 6>; \
	using PIN_NAME_PREFIX ## 7 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 7>; \
	using PIN_NAME_PREFIX ## 8 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 8>; \
	using PIN_NAME_PREFIX ## 9 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 9>; \
	using PIN_NAME_PREFIX ## 10 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 10>; \
	using PIN_NAME_PREFIX ## 11 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 11>; \
	using PIN_NAME_PREFIX ## 12 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 12>; \
	using PIN_NAME_PREFIX ## 13 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 13>; \
	using PIN_NAME_PREFIX ## 14 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 14>; \
	using PIN_NAME_PREFIX ## 15 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 15>; \
	using PIN_NAME_PREFIX ## 16 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 16>; \
	using PIN_NAME_PREFIX ## 17 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 17>; \
	using PIN_NAME_PREFIX ## 18 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 18>; \
	using PIN_NAME_PREFIX ## 19 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 19>; \
	using PIN_NAME_PREFIX ## 20 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 20>; \
	using PIN_NAME_PREFIX ## 21 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 21>; \
	using PIN_NAME_PREFIX ## 22 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 22>; \
	using PIN_NAME_PREFIX ## 23 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 23>; \
	using PIN_NAME_PREFIX ## 24 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 24>; \
	using PIN_NAME_PREFIX ## 25 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 25>; \
	using PIN_NAME_PREFIX ## 26 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 26>; \
	using PIN_NAME_PREFIX ## 27 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 27>; \
	using PIN_NAME_PREFIX ## 28 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 28>; \
	using PIN_NAME_PREFIX ## 29 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 29>; \
	using PIN_NAME_PREFIX ## 30 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 30>; \
	using PIN_NAME_PREFIX ## 31 ## Inv = Zhele::IO::InvertedPin<PORT_TYPE_NAME, 31>; \

	//Short pin definitions
	#if defined (GPIOA)
		DECLARE_PORT_PINS(Porta, Pa)
	#endif

	#if defined (GPIOB)
		DECLARE_PORT_PINS(Portb, Pb)
	#endif

	#if defined (GPIOC)
		DECLARE_PORT_PINS(Portc, Pc)
	#endif

	#if defined (GPIOD)
		DECLARE_PORT_PINS(Portd, Pd)
	#endif

	#if defined (GPIOE)
		DECLARE_PORT_PINS(Porte, Pe)
	#endif

	#if defined (GPIOF)
		DECLARE_PORT_PINS(Portf, Pf)
	#endif

	#if defined (GPIOG)
		DECLARE_PORT_PINS(Portg, Pg)
	#endif
}

#endif // !ZHELE_IOPINS_COMMON_H