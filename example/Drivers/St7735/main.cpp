#include <zhele/clock.h>
#include <zhele/spi.h>
#include <zhele/drivers/st7735.h>
#include <zhele/drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Drivers;


#if defined (STM32G0)
using Interface = Spi1<Dma1Channel1, Dma1Channel2>;
#else
using Interface = Spi1;
#endif

using Lcd = St7735<Interface, IO::Pa4, IO::Pa3, IO::Pa2, 160, 128>;

void ConfigurePins();
void ConfigureSpi();

int main()
{
    ConfigurePins();
    ConfigureSpi();

    // Init display
    Lcd::Init();
    // Fill with black color
    Lcd::FillScreen(Lcd::Color::Black);
    // Wait for complete (fill operation is async)
    while(Lcd::Busy()) continue;

    // Write string with non-monospace font
    Lcd::WriteString<TimesNewRoman13>(10, 10, "Abcdefghijklmnopqrstuvwxyz", Lcd::Color::White, Lcd::Color::Black);
    // Write string with monospace font
    Lcd::WriteString<Fixed10x15Bold>(10, 30, "Abcdefghijklmnopqrstuvwxyz", Lcd::Color::Yellow, Lcd::Color::Black);

    for (;;)
    {
    }
}

void ConfigurePins()
{
    Pa4::Port::Enable();
    Pa4::SetConfiguration(Pa4::Configuration::Out);
    Pa4::SetDriverType(Pa4::DriverType::PushPull);
    Pa4::SetSpeed(Pa4::Speed::Fast);
    Pa4::Set();

    Pa3::Port::Enable();
    Pa3::SetConfiguration(Pa3::Configuration::Out);
    Pa3::SetDriverType(Pa3::DriverType::PushPull);
    Pa3::SetSpeed(Pa3::Speed::Fast);
    Pa3::Clear();

    Pa2::Port::Enable();
    Pa2::SetConfiguration(Pa2::Configuration::Out);
    Pa2::SetDriverType(Pa2::DriverType::PushPull);
    Pa2::SetSpeed(Pa2::Speed::Fast);
    Pa2::Clear();
}

void ConfigureSpi()
{
    Interface::Init(Interface::ClockDivider::Fastest);
    Interface::SetClockPolarity(Interface::ClockPolarity::ClockPolarityHigh);
    Interface::SetClockPhase(Interface::ClockPhase::ClockPhaseFallingEdge);
    Interface::SelectPins<IO::Pa7, IO::Pa6, IO::Pa5, IO::NullPin>();
}