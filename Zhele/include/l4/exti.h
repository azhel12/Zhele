/**
 * @file
 * Implements EXTI for stm32l4 series
 * 
 * @author Alexey Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_EXTI_H
#define ZHELE_EXTI_H

#include <stm32l4xx.h>

#include <common/exti.h>

namespace Zhele
{
    template<uint8_t _Line, IRQn_Type _IRQn>
    template <typename port>
    void Exti<_Line, _IRQn>::SelectPort()
    {
        SYSCFG->EXTICR[_Line / 4] = (SYSCFG->EXTICR[_Line / 4] & (0xF << _Line % 4 * 4)) | ((port::Id - 'A') << (_Line % 4 * 4));
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::SelectPort(uint8_t portID)
    {
        SYSCFG->EXTICR[_Line / 4] = (SYSCFG->EXTICR[_Line / 4] & (0xF << _Line % 4 * 4)) | ((portID - 'A') << (_Line % 4 * 4));
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::EnableClock()
    {
        Zhele::Clock::SysCfgCompClock::Enable();
    }

    using Exti0 = Exti<0, EXTI0_IRQn>;

    using Exti1 = Exti<1, EXTI1_IRQn>;

    using Exti2 = Exti<2, EXTI2_IRQn>;

    using Exti3 = Exti<3, EXTI3_IRQn>;

    using Exti4 = Exti<4, EXTI4_IRQn>;

    using Exti5 = Exti<5, EXTI9_5_IRQn>;
    using Exti6 = Exti<6, EXTI9_5_IRQn>;
    using Exti7 = Exti<7, EXTI9_5_IRQn>;
    using Exti8 = Exti<8, EXTI9_5_IRQn>;
    using Exti9 = Exti<9, EXTI9_5_IRQn>;

    using Exti10 = Exti<10, EXTI15_10_IRQn>;
    using Exti11 = Exti<11, EXTI15_10_IRQn>;
    using Exti12 = Exti<12, EXTI15_10_IRQn>;
    using Exti13 = Exti<13, EXTI15_10_IRQn>;
    using Exti14 = Exti<14, EXTI15_10_IRQn>;
    using Exti15 = Exti<15, EXTI15_10_IRQn>;
}
#endif //! ZHELE_EXTI_H