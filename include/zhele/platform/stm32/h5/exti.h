/**
 * @file
 * Implements EXTI for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_EXTI_H
#define ZHELE_PLATFORM_STM32_H5_EXTI_H

#include <stm32h5xx.h>

// For compatibility with "default" CMSIS (F0/F1/F4)
#define RTSR RTSR1
#define FTSR FTSR1
#define IMR IMR1

#include "../common/exti.h"

namespace Zhele
{
    template<uint8_t _Line, IRQn_Type _IRQn>
    template <typename port>
    void Exti<_Line, _IRQn>::SelectPort()
    {
        EXTI->EXTICR[_Line / 4] = (EXTI->EXTICR[_Line / 4] & ~(0xFu << (_Line % 4 * 8)))
            | ((port::Id - 'A') << (_Line % 4 * 8));
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::SelectPort(uint8_t portID)
    {
        EXTI->EXTICR[_Line / 4] = (EXTI->EXTICR[_Line / 4] & ~(0xFu << (_Line % 4 * 8)))
            | ((portID - 'A') << (_Line % 4 * 8));
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::EnableClock()
    {
        // EXTI is always clocked on H5
    }

    using Exti0 = Exti<0, EXTI0_IRQn>;
    using Exti1 = Exti<1, EXTI1_IRQn>;
    using Exti2 = Exti<2, EXTI2_IRQn>;
    using Exti3 = Exti<3, EXTI3_IRQn>;
    using Exti4 = Exti<4, EXTI4_IRQn>;
    using Exti5 = Exti<5, EXTI5_IRQn>;
    using Exti6 = Exti<6, EXTI6_IRQn>;
    using Exti7 = Exti<7, EXTI7_IRQn>;
    using Exti8 = Exti<8, EXTI8_IRQn>;
    using Exti9 = Exti<9, EXTI9_IRQn>;
    using Exti10 = Exti<10, EXTI10_IRQn>;
    using Exti11 = Exti<11, EXTI11_IRQn>;
    using Exti12 = Exti<12, EXTI12_IRQn>;
    using Exti13 = Exti<13, EXTI13_IRQn>;
    using Exti14 = Exti<14, EXTI14_IRQn>;
    using Exti15 = Exti<15, EXTI15_IRQn>;
}
#endif //! ZHELE_PLATFORM_STM32_H5_EXTI_H
