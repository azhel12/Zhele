// Define target cpu frequence.
#define F_CPU 72000000

#include <clock.h>
#include <spi.h>
#include <drivers/st7735.h>
#include <drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Drivers;

using Lcd = St7735<Spi1, IO::Pa4, IO::Pa3, IO::Pa2, 160, 128>;

void ConfigureClock();
void ConfigurePins();
void ConfigureSpi();

int main()
{
    ConfigureClock();
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

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetMultiplier(9);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
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
    Spi1::Init(Spi1::ClockDivider::Fastest);
    Spi1::SetClockPolarity(Spi1::ClockPolarity::ClockPolarityHigh);
    Spi1::SetClockPhase(Spi1::ClockPhase::ClockPhaseFallingEdge);
    Spi1::SelectPins<IO::Pa7, IO::Pa6, IO::Pa5, IO::NullPin>();
}