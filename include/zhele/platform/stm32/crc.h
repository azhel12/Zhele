/**
 * @file
 * STM32: family dispatch for CRC32
 */

#pragma once

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

#include "common/crc.h"

#include "clock.h"

namespace Zhele {
    using Crc = Private::Crc32<Clock::CrcClock>;
}


