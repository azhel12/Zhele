// Define target cpu frequence.
#define F_CPU 8000000

#include <iopins.h>

using namespace Zhele::IO;

// I have stm32f103c8t6-based board. It has led on C13
int main()
{
	// Enable port clocking
	Pc13::Port::Enable();

	// Configure as out pin
	Pc13::SetConfiguration(Pc13::Configuration::Out);
	// Set driver type as push-pull
	Pc13::SetDriverType(Pc13::DriverType::PushPull);
	// Clear pin (and turn on led)
	Pc13::Clear();

	// Also we can present C13 as inverted
	// Turn on
	Pc13Inv::Set();

	for (;;)
	{
	}
}