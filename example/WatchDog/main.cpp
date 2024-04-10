#include <zhele/iopins.h>
#include <zhele/timer.h>
#include <zhele/watchdog.h>

using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Timers;

using Led = Pc13Inv;
int main()
{
    Led::Port::Enable();
    Led::SetConfiguration(Led::Configuration::Out);
    Led::SetDriverType(Led::DriverType::PushPull);
    Led::Clear();

    Timer3::Enable();
    Timer3::SetPrescaler(3999);
    Timer3::SetPeriod(1999);
    Timer3::EnableInterrupt();
    Timer3::Start();
    
    LsiClock::Enable();
    //IWdg::Start(1000); // Led is turn off (watchdog resets MCU before timer's interrupt handler)
    IWdg::Start(1300); // Led is blinking

    for (;;)
    {
    }
}

extern "C"
{
    void TIM3_IRQHandler()
    {
        Led::Toggle();
        Timer3::ClearInterruptFlag();
        IWdg::Reset();
    }
}

// RAM: 28; Flash: 440