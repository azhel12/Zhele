/**
 * @file
 * Containing template classes, which implements MCU clock.
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#include <f1/clock.h>

namespace Zhele
{
	typedef uint32_t clock_freq_t;
	namespace Clock
	{	
		clock_freq_t PllClock::GetDivider()
		{
			if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0)
				return 2;
			else
			#if defined(RCC_CFGR2_PREDIV1)
				return (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1;
			#else
				return ((RCC->CFGR & RCC_CFGR_PLLXTPRE) >> 17) + 1;
			#endif
		}
		
		clock_freq_t PllClock::GetMultipler()
		{
		#if defined(RCC_CFGR_PLLMULL6_5)
			clock_freq_t mul = ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18);
			if(mul == 13) return 6;
			return mul + 2;
		#else
			return ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18) + 2;
		#endif
		}
		
		void PllClock::SetMultipler(uint8_t multiplier)
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
			RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLMULL) | (multiplier << 18);
		}
		
		void PllClock::SetDivider(uint8_t divider)
		{
			#if defined(RCC_CFGR2_PREDIV1)
				if(divider > 15)
					divider = 15;
				divider-=1;
				RCC->CFGR2 = (RCC->CFGR2 & ~RCC_CFGR2_PREDIV1) | (divider);
			#else
				if(divider > 2)
					divider = 2;
				divider-=1;
				RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLXTPRE) | (divider << 17);
			#endif
		}
		
		clock_freq_t PllClock::SetClockFreq(clock_freq_t freq)
		{
			if(freq > SysClock::MaxFreq())
				freq = SysClock::MaxFreq();
							
			uint32_t resPllDiv = 0;
			uint32_t resPllMul = 0;
			
			uint32_t srcFreq = SrcClockFreq();
			
			#if defined(RCC_CFGR2_PREDIV1)
				resPllDiv = 16;
			#else
				resPllDiv = 2;
			#endif
			
			while(resPllDiv > 1)
			{
				uint32_t pllMul = resPllDiv * freq / srcFreq;
				
				#if defined(RCC_CFGR2_PREDIV1)
				if(pllMul <= 9 && pllMul >= 4)
				{
					resPllMul = pllMul;
					break;
				}
				#else
				if(pllMul <= 16)
				{
					resPllMul = pllMul;
					break;
				}
				#endif
				resPllDiv--;
			}
			
			SetDivider(resPllDiv);
			SetMultipler(resPllMul);
			
			return ClockFreq();
		}
		
		SysClock::ErrorCode SysClock::SelectClockSource(ClockSource clockSource)
		{
			uint32_t clockStatusValue;
			uint32_t clockSelectMask;
			
			if(clockSource == Internal)
			{
				clockStatusValue = RCC_CFGR_SWS_HSI;
				clockSelectMask = RCC_CFGR_SW_HSI;
				if (!HsiClock::Enable())
					return ClockSourceFailed;
			}else if(clockSource == External)
			{
				clockStatusValue = RCC_CFGR_SWS_HSE;
				clockSelectMask = RCC_CFGR_SW_HSE;
				if (!HseClock::Enable())
					return ClockSourceFailed;
			}else if(clockSource == Pll)
			{
				clockStatusValue = RCC_CFGR_SWS_PLL;
				clockSelectMask = RCC_CFGR_SW_PLL;
				if (!PllClock::Enable())
					return ClockSourceFailed;

				FLASH->ACR |= FLASH_ACR_PRFTBE;
				FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
				#if F_CPU <= 24000000
					FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;
				#elif F_CPU <= 48000000
					FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_1;
				#else
					FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;
				#endif

			} else
				return InvalidClockSource;

			RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | clockSelectMask;
			
			uint32_t timeout = 10000;
			while (((RCC->CFGR & RCC_CFGR_SWS) != clockStatusValue) && --timeout);
			if(timeout == 0)
			{
				return ClockSelectFailed;
			}
			return Success;
		}

		clock_freq_t SysClock::MaxFreq()
		{
			return 72000000;
		}
	}
}