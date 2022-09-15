/**
 * @file
 * Implements clock methods.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#include <clock.h>

namespace Zhele::Clock
{
    constexpr ClockFrequenceT HseClock::SrcClockFreq()
    {
        return HSE_VALUE;
    }

    constexpr ClockFrequenceT HseClock::GetDivider() { return 1; }

    constexpr ClockFrequenceT HseClock::GetMultipler() { return 1; }

    constexpr ClockFrequenceT HseClock::ClockFreq()
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

    constexpr ClockFrequenceT HsiClock::SrcClockFreq()
    {
        return HSI_VALUE;
    }
    constexpr ClockFrequenceT HsiClock::GetDivider() { return 1; }

    constexpr ClockFrequenceT HsiClock::GetMultipler() { return 1; }

    constexpr ClockFrequenceT HsiClock::ClockFreq()
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
#if defined (RCC_PLLCFGR_PLLP)
        uint8_t pllpValue = GetSystemOutputDivider() == SystemOutputDivider::Div2
            ? 2
            : GetSystemOutputDivider() == SystemOutputDivider::Div4
                ? 4
                : GetSystemOutputDivider() == SystemOutputDivider::Div6
                    ? 6
                    : 8;
        return SrcClockFreq() / GetDivider() * GetMultipler() / pllpValue;
#else
        return SrcClockFreq() / GetDivider() * GetMultipler();
#endif
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