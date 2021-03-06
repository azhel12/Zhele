/**
 * @file
 * Implements clock methods.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#include "common/clock.h"

namespace Zhele::Clock
{
    ClockFrequenceT HseClock::SrcClockFreq()
    {
        return HSE_VALUE;
    }

    ClockFrequenceT HseClock::GetDivider() { return 1; }

    ClockFrequenceT HseClock::GetMultipler() { return 1; }

    ClockFrequenceT HseClock::ClockFreq()
    {
        return SrcClockFreq();
    }

    bool HseClock::Enable()
    {
        return ClockBase::EnableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
    }

    bool HseClock::Disable()
    {
        return ClockBase::DisableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
    }

    ClockFrequenceT HsiClock::SrcClockFreq()
    {
        return HSI_VALUE;
    }
    ClockFrequenceT HsiClock::GetDivider() { return 1; }

    ClockFrequenceT HsiClock::GetMultipler() { return 1; }

    ClockFrequenceT HsiClock::ClockFreq()
    {
        return SrcClockFreq();
    }

    bool HsiClock::Enable()
    {
        return ClockBase::EnableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
    }

    bool HsiClock::Disable()
    {
        return ClockBase::DisableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
    }

    ClockFrequenceT PllClock::SrcClockFreq()
    {
        return GetClockSource() == External
            ? HseClock::ClockFreq()
            : HsiClock::ClockFreq();
    }

    ClockFrequenceT PllClock::ClockFreq()
    {
        return SrcClockFreq() / GetDivider() * GetMultipler();
    }

    bool PllClock::Enable()
    {
        if (GetClockSource() == Internal)
        {
            if (!HsiClock::Enable())
                return false;
        }
        else
        {
            if (!HseClock::Enable())
                return false;
        }
        return ClockBase::EnableClockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
    }

    void PllClock::Disable()
    {
        ClockBase::DisableClockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
    }
#if defined (RCC_CSR_LSION)
    ClockFrequenceT LsiClock::SrcClockFreq()
    {
        return 32768;
    }

    ClockFrequenceT LsiClock::GetDivider() { return 1; }

    ClockFrequenceT LsiClock::GetMultipler() { return 1; }

    ClockFrequenceT LsiClock::ClockFreq()
    {
        return SrcClockFreq();
    }

    bool LsiClock::Enable()
    {
        return ClockBase::EnableClockSource(RCC_CSR_LSION, RCC_CSR_LSIRDY);
    }

    bool LsiClock::Disable()
    {
        return ClockBase::DisableClockSource(RCC_CSR_LSION, RCC_CSR_LSIRDY);
    }
#endif
    SysClock::ErrorCode SysClock::SelectClockSource(ClockSource clockSource)
    {
        uint32_t clockStatusValue;
        uint32_t clockSelectMask;
        uint32_t sourceFrequence;

        if(clockSource == Internal)
        {
            clockStatusValue = RCC_CFGR_SWS_HSI;
            clockSelectMask = RCC_CFGR_SW_HSI;
            if (!HsiClock::Enable())
                return ClockSourceFailed;
            sourceFrequence = HsiClock::ClockFreq();
        }
        else if(clockSource == External)
        {
            clockStatusValue = RCC_CFGR_SWS_HSE;
            clockSelectMask = RCC_CFGR_SW_HSE;
            if (!HseClock::Enable())
                return ClockSourceFailed;
            sourceFrequence = HseClock::ClockFreq();
        }
        else if(clockSource == Pll)
        {
            clockStatusValue = RCC_CFGR_SWS_PLL;
            clockSelectMask = RCC_CFGR_SW_PLL;
            if (!PllClock::Enable())
                return ClockSourceFailed;
            sourceFrequence = PllClock::ClockFreq();
        }
        else
        {
            return InvalidClockSource;
        }

        Flash::ConfigureFrequence(sourceFrequence);

        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | clockSelectMask;
        
        uint32_t timeout = 10000;
        while (((RCC->CFGR & RCC_CFGR_SWS) != clockStatusValue) && --timeout)
            ;
        if(timeout == 0)
        {
            return ClockSelectFailed;
        }
        return Success;
    }

    ClockFrequenceT SysClock::ClockFreq()
    {
        uint32_t clockSrc = RCC->CFGR & RCC_CFGR_SWS;
        switch (clockSrc)
        {
            case 0:              return HsiClock::ClockFreq();
            case RCC_CFGR_SWS_0: return HseClock::ClockFreq();
            case RCC_CFGR_SWS_1: return PllClock::ClockFreq();
        }
        return 0;
    }

    ClockFrequenceT SysClock::SrcClockFreq()
    {
        return ClockFreq();
    }

    ClockFrequenceT SysClock::SetClockFreq(ClockFrequenceT freq)
    {
        SelectClockSource(Internal);
        PllClock::Disable();
        PllClock::SelectClockSource(PllClock::External);
        PllClock::SetClockFreq(freq);
        SelectClockSource(Pll);
        return ClockFreq();
    }
}