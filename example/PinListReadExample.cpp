#define F_CPU 8000000

#include <iopins.h>
#include <pinlist.h>

using namespace Zhele::IO;

int main()
{
    // Declare pin list
	using virtualPort = PinList<Pb0, Pa1, Pa0, Pc15, Pc14>;

    // Enable virtual port (all used real ports)
    virtualPort::Enable();

    // Set configuration to out
    virtualPort::SetConfiguration<virtualPort::Configuration::In>();

    // Set pull mode type to pull down
    virtualPort::SetPullMode<virtualPort::PullMode::PullDown>();
    
    // Read port's value
    volatile auto portValue = virtualPort::Read();
    
    // In my example B0, A0 and C15 is 3.3v, so portValue is equal 13. (0b01101)
    volatile auto a = 42; // for debug breakpoint

	for (;;)
	{
	}
}