#include <zhele/iopins.h>

using namespace Zhele::IO;

// Connect LED on A4 for this example (or edit example code, it's simple).
int main()
{
	// Enable port clocking
	Pa4::Port::Enable();

	// Configure as out pin
	Pa4::SetConfiguration(Pa4::Configuration::Out);
	// Set driver type as push-pull
	Pa4::SetDriverType(Pa4::DriverType::PushPull);
	// Clear pin (and turn on led)
	Pa4::Clear();

	for (;;)
	{
	}
}