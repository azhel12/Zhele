// Define target cpu frequence. Used for delay, configure flash latency
#define F_CPU 72000000

#include <clock.h>

using namespace Zhele::Clock;

int main()
{
	// Select HSE as PLL source	
	PllClock::SelectClockSource(PllClock::External);
	// Set PLL multiplier
	PllClock::SetMultiplier(9);
	// Set Apb1 prescaler
	Apb1Clock::SetPrescaler(Apb1Clock::Div2);
	// Try select PLL as sysclock source
	SysClock::SelectClockSource(SysClock::Pll);

	for (;;)
	{
	}
}