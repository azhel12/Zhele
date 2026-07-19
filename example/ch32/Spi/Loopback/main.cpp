/**
 * SPI1 loopback self-test — jumper MOSI to MISO. Master sends a byte pattern and
 * checks each byte comes back; the LED shows the result:
 *   solid ON  = all bytes echoed (loopback OK)
 *   blinking  = mismatch (no jumper / wiring issue)
 *
 *   V003/V006: SCK=PC5 MISO=PC7 MOSI=PC6  (jumper PC6<->PC7), LED=PC4
 *   V203:      SCK=PA5 MISO=PA6 MOSI=PA7  (jumper PA7<->PA6), LED=PB2
 */
#include <zhele/spi.h>
#include <zhele/iopins.h>
#include <zhele/delay.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32V203)
using Led  = Pb2;
using Sck  = Pa5;
using Miso = Pa6;
using Mosi = Pa7;
#else
using Led  = Pc4;
using Sck  = Pc5;
using Miso = Pc7;
using Mosi = Pc6;
#endif

int main()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    Spi1::Init(Spi1::Medium, Spi1::Master);
    Spi1::SelectPins<Mosi, Miso, Sck>();

    const uint8_t pattern[] = {0xA5, 0x3C, 0xFF, 0x00, 0x5A};
    bool ok = true;
    for (uint8_t v : pattern)
        if ((Spi1::Send(v) & 0xFF) != v)
            ok = false;

    if (ok) {
        Led::Set(); // solid on = loopback OK
        for (;;) {
        }
    }

    for (;;) { // fast blink = mismatch
        Led::Toggle();
        delay_ms<150>();
    }
}
