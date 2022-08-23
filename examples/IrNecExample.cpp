#define F_CPU 72000000

#include <clock.h>
#include <iopins.h>
#include <timer.h>
#include <drivers/ir.h>

using namespace Zhele;
using namespace Zhele::Drivers;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Clock;

void ConfigureClock();
void ConfigureOutputPwm();
void ConfigureInputCapture();

using Receiver = IrReceiver<Timer4, Pb6, NecDecoder>;

int main()
{    
    ConfigureClock();

    Receiver::Init();
    NecDecoder::SetCallback([](NecDecoder::Command command) {
        // Do smth
    });

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

extern "C"
{
    void TIM4_IRQHandler()
    {
        Receiver::IRQHandler();
    }
}