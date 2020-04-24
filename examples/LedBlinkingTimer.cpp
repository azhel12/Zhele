#include <iopins.h>
#include <timer.h>

using namespace Zhele::IO;
using namespace Zhele::Timers;

int main()
{
	// Enable LED (C13 GPIO)
	Pc13::Port::Enable();
	Pc13::SetConfiguration(Pc13::Configuration::Out);
	Pc13::SetDriverType(Pc13::DriverType::PushPull);

	// Enable timer (enable timer`s clock)
	Timer2::Enable();
	// Set timer prescaler. There is no custom clock configuration in this example, so MCU works on 8Mhz
	Timer2::SetPrescaler(3999);
	// Set period. With CPU = 8Mhz timer will tick every 1 sec (8Mhz/4000/2000)
	Timer2::SetPeriod(1999);
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
	void TIM2_IRQHandler()
	{
		Pc13::Toggle();
		Timer2::ClearInterruptFlag();
	}
}