/**
 * GPIO blink — toggle a single LED with a millisecond delay (V003 / V006 / V203 / X035).
 *
 * LED pin (adjust to your board):
 *   CH32V003 / CH32V006 -> PC4
 *   CH32V203            -> PB2
 *   CH32X035            -> PB12
 */
#include <zhele/iopins.h>
#include <zhele/delay.h>
#include <zhele/clock.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32V203)
using Led = Pb2;
#elif defined(CH32X035)
using Led = Pb12;
#else
using Led = Pc4;
#endif

int main()
{
#if defined(CH32X035)
    Clock::AhbClock::SetPrescaler<Clock::AhbClock::Div1>();
#endif
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    for (;;) {
        Led::Toggle();
        delay_ms<500>();
    }
}
