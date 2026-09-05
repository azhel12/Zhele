#include <zhele/iopins.h>
#include <zhele/timer.h>
#if defined (STM32H5)
#include <zhele/clock.h>
#endif

using namespace Zhele::IO;
using namespace Zhele::Timers;

//using Timer3 = Timer3; // Only for stm32f0. For Zhele example.

#if defined (STM32H5)
	// STM32H503CB: LED on PC13. These pins sit behind the backup-domain power
	// switch: 2 MHz output speed limit, and ST states they must not source
	// current for an LED (DS14053 table 10).
	using Led = Pc13;
#else
	using Led = Pa4;
#endif

int main()
{
#if defined (STM32H5)
	// Run SYSCLK straight off HSI with no divider (64 MHz); reset default is /2.
	// Timer clock follows APB1: with TIMPRE = 0 and PPRE1 = /1 it is the full
	// 64 MHz. Careful -- at PPRE1 = /2 or slower the timers get 2 x pclk1.
	Zhele::Clock::SetHsiDivider<1>();
	Zhele::Clock::SysClock::SelectClockSource<Zhele::Clock::SysClock::Internal>();
#endif

	// Enable LED
	Led::Port::Enable();
	Led::SetConfiguration(Led::Configuration::Out);
	Led::SetDriverType(Led::DriverType::PushPull);
#if defined (STM32H5)
	Led::SetSpeed(Led::Port::Speed::Slow);
#endif
	Led::Set();

	// Enable timer (enable timer`s clock)
	Timer3::Enable();
#if defined (STM32H5)
	// 64 MHz / 64000 / 500 = 2 Hz update event -> ~1 Hz blink
	Timer3::SetPrescaler(64000 - 1);
	Timer3::SetPeriod(500 - 1);
#else
	// Set timer prescaler. There is no custom clock configuration in this example, so MCU works on 8Mhz (24 for my stm32f401)
	Timer3::SetPrescaler(3999);
	// Set period. With CPU = 8Mhz timer will tick every 1 sec (8Mhz/4000/2000)
	Timer3::SetPeriod(1999);
#endif
	// Enable interrupt
	Timer3::EnableInterrupt();
	// Start timer
	Timer3::Start();

	for (;;)
	{
	}
}

extern "C"
{
	// Toogle led on timer`s tick
	void TIM3_IRQHandler()
	{
		Led::Toggle();
		Timer3::ClearInterruptFlag();
	}
}