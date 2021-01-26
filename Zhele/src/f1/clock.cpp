#if defined (STM32F1)
    #include "../include/f1/clock.h"

namespace Zhele::Clock
{
    ClockFrequenceT PllClock::SetClockFreq(ClockFrequenceT freq)
    {
        // Not implemented yet
        return 0;
    }

    ClockFrequenceT PllClock::GetDivider()
    {
        if(GetClockSource() == Internal)
            return 2;
    #if defined (RCC_CFGR_PLLXTPRE)
        return (RCC->CFGR & RCC_CFGR_PLLXTPRE_HSE_DIV2) > 0 
            ? 2
            : 1;
    #else
        return ((RCC->CFGR2 & RCC_CFGR2_PREDIV1) >> RCC_CFGR2_PREDIV1_Pos) + 1;
    #endif
    }

    void PllClock::SetDivider(ClockFrequenceT divider)
    {
    #if defined(RCC_CFGR2_PREDIV1)
        if(divider > 15)
            divider = 15;
        divider -= 1;
        RCC->CFGR2 = (((RCC->CFGR2 & ~RCC_CFGR2_PREDIV1) | (divider << RCC_CFGR2_PREDIV1_Pos));
    #else
        if(divider > 2)
            divider = 2;
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLXTPRE) | (divider == 2
            ? RCC_CFGR_PLLXTPRE_HSE_DIV2
            : RCC_CFGR_PLLXTPRE_HSE);
    #endif
    }

    ClockFrequenceT PllClock::GetMultipler()
    {
    #if defined(RCC_CFGR_PLLMULL6_5)
        clock_freq_t mul = ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18);
        if(mul == 13) return 65;
        return mul + 2;
    #else
        return ((RCC->CFGR & RCC_CFGR_PLLMULL) >> RCC_CFGR_PLLMULL_Pos) + 2;
    #endif
    }

    void PllClock::SetMultiplier(ClockFrequenceT multiplier)
    {
    #if !(defined(RCC_CFGR_PLLMULL3) && defined(RCC_CFGR_PLLMULL10))
        if(multiplier > 9)
            multiplier = 9;
        if(multiplier < 4)
            multiplier = 4;
    #else
        if(multiplier > 16)
            multiplier = 16;
    #endif
        multiplier-=2;
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLMULL) | (multiplier << RCC_CFGR_PLLMULL_Pos);
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
