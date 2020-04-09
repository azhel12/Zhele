/**
 * @file
 * Containing template classes, which implements MCU clock.
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#include <clock.h>

namespace Zhele
{
	typedef uint32_t clock_freq_t;
	namespace Clock
	{
		bool ClockBase::EnableClockSource(unsigned turnOnMask,  unsigned waitReadyMask)
		{
			uint32_t timeoutCounter = 0;
			RCC->CR |= turnOnMask;
			while(((RCC->CR & waitReadyMask) == 0) && (timeoutCounter < ClockStartTimeout))
			{
				timeoutCounter++;
			}
			return (RCC->CR & waitReadyMask) != 0;
		}
		
		bool ClockBase::DisableClockSource(unsigned turnOnMask,  unsigned waitReadyMask)
		{
			uint32_t timeoutCounter = 0;
			RCC->CR &= ~turnOnMask;
			while(((RCC->CR & waitReadyMask) != 0) && (timeoutCounter < ClockStartTimeout))
			{
				timeoutCounter++;
			}
			return (RCC->CR & waitReadyMask) == 0;
		}
		
		clock_freq_t PllClock::SrcClockFreq()
		{
			return (RCC->CFGR & RCC_CFGR_PLLSRC) == 0
				? HsiClock::ClockFreq()
				: HseClock::ClockFreq();
		}
		
		void PllClock::SelectClockSource(ClockSource clockSource)
		{
			RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLSRC) | clockSource;
		}
		
		clock_freq_t PllClock::ClockFreq()
		{
			return SrcClockFreq() / GetDivider() * GetMultipler();
		}
		
		bool PllClock::Enable()
		{
			if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0)
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
		
		clock_freq_t SysClock::ClockFreq()
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
		
		
		clock_freq_t SysClock::SetClockFreq(clock_freq_t freq)
		{
			SelectClockSource(Internal);
			PllClock::Disable();
			PllClock::SelectClockSource(PllClock::External);
			PllClock::SetClockFreq(freq);
			SelectClockSource(Pll);
			return ClockFreq();
		}
	}
}