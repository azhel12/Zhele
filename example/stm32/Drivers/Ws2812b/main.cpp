#include <zhele/delay.h>
#include <zhele/iopins.h>
#include <zhele/spi.h>
#include <zhele/drivers/ws2812b.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Drivers;

#if defined (STM32G0)
#include <zhele/dma.h>
#include <zhele/dmamux.h>
using Interface = Spi1<Dma1Channel1, Dma1Channel2>;
#else
using Interface = Spi1;
#endif

// Strip of 8 LEDs on MOSI (Pa7).
//
// The examples run on the reset default clock tree, so SPI1 sees the raw HSI: 8 MHz on
// F0/F1 and 16 MHz on F4/G0. That is the only frequence stated here - the driver derives
// the prescaler (/2 and /4 respectively, both give 4 MHz), the encoding and the reset
// length from it, and refuses to compile if nothing fits the strip.
#if defined (STM32F0) || defined (STM32F1)
constexpr unsigned SpiSourceClockHz = 8'000'000;
#else
constexpr unsigned SpiSourceClockHz = 16'000'000;
#endif

using Strip = Ws2812b<Interface, 8, SpiSourceClockHz>;

int main()
{
#if defined (STM32G0)
    Dma1::Enable();
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::Spi1Tx);
#endif

    Strip::Init();
    // The strip needs MOSI only, but SCK is driven by the peripheral anyway
    Interface::SelectPins<Pa7, NullPin, Pa5, NullPin>();

    // Turn everything off
    Strip::Flush();

    // Run a rainbow along the strip
    uint8_t hue = 0;
    for (;;)
    {
        for (unsigned i = 0; i < Strip::Count(); ++i) {
            const auto pixelHue = static_cast<uint8_t>(hue + i * 256 / Strip::Count());
            // These LEDs are bright, one fifth is more than enough for a demo
            Strip::SetPixel(i, Strip::Color::FromHsv(pixelHue, 255, 48));
        }

        Strip::Flush();

        delay_ms<20>();
        ++hue;
    }
}
