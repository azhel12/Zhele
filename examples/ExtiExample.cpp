#include <exti.h>
#include <iopins.h>

using namespace Zhele;
using namespace Zhele::IO;

int main()
{
	// Enable LED (A7 GPIO)
	Pa7::Port::Enable();
	Pa7::SetConfiguration(Pa7::Configuration::Out);
	Pa7::SetDriverType(Pa7::DriverType::PushPull);
    // And turn it on
    Pa7::Set();


    // Configure A0 as EXTI source. Trigger on rising.
    Exti0::Init<Exti0::Trigger::Rising, Porta>();
    
    // Or you can choose port later:
    // Exti0::SelectPort<Porta>();
    // Or you can do this other way: Exti0::SelectPort('A');

    // Configure A0 pin
    // You can do it without using Exti class
    Exti0::InitPin<Pa0>(Pa0::PullMode::PullDown);

    // Enable interrupt
    Exti0::EnableInterrupt();


	for (;;)
	{
	}
}


extern "C"
{
	// Toogle led on timer`s tick
	void EXTI0_IRQHandler() // "void EXTI0_1_IRQHandler()" for Stm32F0
	{
		Pa7::Toggle();
		Exti0::ClearInterruptFlag();
	}
}