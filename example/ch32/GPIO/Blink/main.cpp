/**
 * GPIO blink — toggle a single LED with a millisecond delay (V003 / V006 / V203).
 *
 * LED pin (adjust to your board):
 *   CH32V003 / CH32V006 -> PC4
 *   CH32V203            -> PB2
 */
#include <zhele/iopins.h>
#include <zhele/delay.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32V203)
using Led = Pb2;
#else
using Led = Pc4;
#endif

int main()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    for (;;) {
        Led::Toggle();
        delay_ms<500>();
    }
}
