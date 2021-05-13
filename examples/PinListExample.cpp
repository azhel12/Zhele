// Define target cpu frequence.
#define F_CPU 8000000

#include <iopins.h>
#include <pinlist.h>

using namespace Zhele::IO;

// In this example I declare pinlist with pins of all GPIO ports (A, B, C)
// write some value and check by multimeter. You can use LEDs.
int main()
{
    // Declare pin list
	using virtualPort = PinList<Pb0, Pa1, Pa0, Pc15, Pc14>;

    // Enable virtual port (all used real ports)
    virtualPort::Enable();

    // Set configuration to out
    virtualPort::SetConfiguration<virtualPort::Configuration::Out>();

    // Set dirver type to push-pull
    virtualPort::SetDriverType<virtualPort::DriverType::PushPull>();

    // Set pull mode type to pull down
    virtualPort::SetPullMode<virtualPort::PullMode::PullDown>();

    // Write 0 to pinlist
    // virtualPort::Write(0x00);
    // You can belive me that all pins are 0v=)

    // Write 0b10101
    virtualPort::Write(0b10101);
    // You can belive me that Pb0, Pa0 and Pc14 voltage is 3.3v
	for (;;)
	{
	}
}