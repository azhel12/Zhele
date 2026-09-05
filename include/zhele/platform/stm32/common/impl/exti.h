/**
 * @file
 * EXTI methods implementation
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_STM32_COMMON_IMPL_EXTI_H
#define ZHELE_PLATFORM_STM32_COMMON_IMPL_EXTI_H

#include <zhele/clock.h>
#include <zhele/ioports.h>

namespace Zhele
{
    template<uint8_t _Line, IRQn_Type _IRQn>
    template <typename Exti<_Line, _IRQn>::Trigger trigger, typename port>
    void Exti<_Line, _IRQn>::Init()
    {
        EnableClock();
        EXTI->RTSR = (EXTI->RTSR & ~(1 << _Line)) | ((trigger & 1) << _Line);
        EXTI->FTSR = (EXTI->FTSR & ~(1 << _Line)) | (((trigger >> 1) & 1) << _Line);
        SelectPort<port>();
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::Init(Trigger trigger, char portId)
    {
        EnableClock();
        EXTI->RTSR = (EXTI->RTSR & ~(1 << _Line)) | ((trigger & 1) << _Line);
        EXTI->FTSR = (EXTI->FTSR & ~(1 << _Line)) | (((trigger >> 1) & 1) << _Line);
        SelectPort(portId);
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    template <typename pin>
    void Exti<_Line, _IRQn>::InitPin(
        typename pin::PullMode pullMode,
        typename pin::DriverType driverType,
        typename pin::Speed speed)
    {
        pin::Port::Enable();
        pin::SetConfiguration(pin::Configuration::In);
        pin::SetPullMode(pullMode);
#if !defined(STM32F1)
        pin::SetDriverType(driverType);
        pin::SetSpeed(speed);
#endif
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::EnableInterrupt()
    {
        EXTI->IMR = (EXTI->IMR & ~(1 << _Line)) | (1 << _Line);
        NVIC_EnableIRQ(_IRQn);
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::DisableInterrupt()
    {
        EXTI->IMR &= (~(1 << _Line));
        NVIC_DisableIRQ(_IRQn);
    }

    template<uint8_t _Line, IRQn_Type _IRQn>
    void Exti<_Line, _IRQn>::ClearInterruptFlag()
    {
#if defined (EXTI_RPR1_RPIF0)
        EXTI->RPR1 = (1 << _Line);
        EXTI->FPR1 = (1 << _Line);
#else
        EXTI->PR |= (1 << _Line);
#endif
    }
}
#endif //! ZHELE_PLATFORM_STM32_COMMON_IMPL_EXTI_H