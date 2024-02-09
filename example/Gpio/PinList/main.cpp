#include <zhele/iopins.h>
#include <zhele/pinlist.h>

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
    // Write 0b10101
    virtualPort::Write(0b10101);
    // You can belive me that Pb0, Pa0 and Pc14 voltage is 3.3v


    // Set configuration to in
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