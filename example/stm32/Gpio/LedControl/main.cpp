#include <zhele/iopins.h>

using namespace Zhele::IO;

// Connect LED on A4 for this example (or edit example code, it's simple).
#if defined (STM32H5)
	// STM32H503CB: LED on PC13. Note PC13..PC15 are fed through the backup-domain
	// power switch, so output speed is capped at 2 MHz and ST states these pins
	// must not source current for an LED -- they are meant to sink it
	// (DS14053 table 10).
	using Led = Pc13;
#else
	using Led = Pa4;
#endif

int main()
{
	// Enable port clocking
	Led::Port::Enable();

	// Configure as out pin
	Led::SetConfiguration(Led::Configuration::Out);
	// Set driver type as push-pull
	Led::SetDriverType(Led::DriverType::PushPull);
#if defined (STM32H5)
	Led::SetSpeed(Led::Port::Speed::Slow);
#endif
	// Clear pin (and turn on led)
	Led::Clear();

	for (;;)
	{
	}
}