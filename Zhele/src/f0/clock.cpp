/**
 * @file
 * Implements clock methods for Stm32f0 series.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#if defined (STM32F0)

#include "../../include/f0/clock.h"

namespace Zhele::Clock
{
    ClockFrequenceT PllClock::SetClockFreq(ClockFrequenceT freq)
    {
        return 0;
    }

    ClockFrequenceT PllClock::GetDivider()
    {
        if(GetClockSource() == Internal)
            return 2;

        return (RCC->CFGR & RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV2) > 0 
            ? 2
            : 1;
    }

    void PllClock::SetDivider(ClockFrequenceT divider)
    {
        if(divider > 2)
            divider = 2;
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLXTPRE) | (divider == 2
            ? RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV2
            : RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1);
    
    }

    ClockFrequenceT PllClock::GetMultipler()
    {
        return ((RCC->CFGR & RCC_CFGR_PLLMUL) >> RCC_CFGR_PLLMUL_Pos) + 2;
    }

    void PllClock::SetMultiplier(ClockFrequenceT multiplier)
    {
        if(multiplier > 16)
            multiplier = 16;
        multiplier-=2;
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLMUL) | (multiplier << RCC_CFGR_PLLMUL_Pos);
    }    

    void PllClock::SelectClockSource(ClockSource clockSource)
    {
        RCC->CFGR = clockSource == External
            ? RCC->CFGR  | RCC_CFGR_PLLSRC
            : RCC->CFGR  & ~RCC_CFGR_PLLSRC;
    }

    PllClock::ClockSource PllClock::GetClockSource()
    {
        return RCC->CFGR & RCC_CFGR_PLLSRC
            ? ClockSource::External
            : ClockSource::Internal;
    }
}

#endif