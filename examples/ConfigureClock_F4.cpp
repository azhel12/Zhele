#define HSE_VALUE 25000000
// Define target cpu frequence. Used for delay, configure flash latency
#define F_CPU 84000000

#include <clock.h>
#include <iopins.h>
#include <timer.h>

using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Timers;

int main()
{
	// Select HSE as PLL source	
	PllClock::SelectClockSource(PllClock::External);
	// Set PLLM value
	PllClock::SetDivider(25);
	// Set PllN value
	PllClock::SetMultiplier(336);
	// Set PLLP
	PllClock::SetSystemOutputDivider(4);
	// Set Apb1 prescaler
	Apb1Clock::SetPrescaler(Apb1Clock::Div2);
	// Try select PLL as sysclock source
	SysClock::SelectClockSource(SysClock::Pll);

	// Enable LED (C13 GPIO)
	Pc13::Port::Enable();
	Pc13::SetConfiguration(Pc13::Configuration::Out);
	Pc13::SetDriverType(Pc13::DriverType::PushPull);
	Pc13::Set();

	// Enable timer (enable timer`s clock)
	Timer2::Enable();
	// Set timer prescaler. There is no custom clock configuration in this example, so MCU works on 84Mhz
	Timer2::SetPrescaler(23999);
	// Set period. With CPU = 84Mhz timer will tick every 1 sec (84Mhz/24000/4000)
	Timer2::SetPeriod(3999);
	// Enable interrupt
	Timer2::EnableInterrupt();
	// Start timer
	Timer2::Start();


	for (;;)
	{
	}
}


extern "C"
{
	// Toogle led on timer`s tick
	// void TIM3_IRQHandler() for stm32f0
	void TIM2_IRQHandler()
	{
		Pc13::Toggle();
		Timer2::ClearInterruptFlag();
	}
}