#include <iopins.h>
#include <timer.h>

using namespace Zhele::IO;
using namespace Zhele::Timers;

//using Timer2 = Timer3; // Only for stm32f0. For Zhele example.

int main()
{
	// Enable LED (C13 GPIO)
	Pa4::Port::Enable();
	Pa4::SetConfiguration(Pa4::Configuration::Out);
	Pa4::SetDriverType(Pa4::DriverType::PushPull);
	Pa4::Set();
	// Enable timer (enable timer`s clock)
	Timer2::Enable();
	// Set timer prescaler. There is no custom clock configuration in this example, so MCU works on 8Mhz (24 for my stm32f401)
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
	// void TIM3_IRQHandler() for stm32f0
	void TIM2_IRQHandler()
	{
		Pa4::Toggle();
		Timer2::ClearInterruptFlag();
	}
}