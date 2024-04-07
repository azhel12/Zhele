/**
 * @file
 * United header for CRC32
 *
 * @author Alexey Zhelonkin
 * @date 2024
 * @licence FreeBSD
 */

#if defined(STM32F0)
    #include <stm32f0xx.h>
#endif
#if defined(STM32F1)
    #include <stm32f1xx.h>
#endif
#if defined(STM32F4)
    #include <stm32f4xx.h>
#endif
#if defined(STM32L4)
    #include <stm32l4xx.h>
#endif
#if defined(STM32G0)
    #include <stm32g0xx.h>
#endif

#include "clock.h"
#include "common/crc.h"

namespace Zhele {
    using Crc = Crc32<Clock::CrcClock>;
}