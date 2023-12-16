/*
I was need to make device that receive PWM signal and out it with some latency.
First solution based on EXTI was ugly, but current code based on timer trigger + input capture + output compare seems very good.
*/
#include <iopins.h>
#include <timer.h>

using DelayTimer = Zhele::Timers::Timer2;
using InputChannel = DelayTimer::InputCapture<0>;
using OuputChannel = DelayTimer::OutputCompare<2>;

using InputLine = Zhele::IO::Pa0;
using OutputLine = Zhele::IO::Pa2;

void ConfigureClock();
void ConfigureTimer();

int main()
{
    ConfigureClock();
    ConfigureTimer();
    for (;;)
    {
    }
}

void ConfigureClock()
{
    using namespace Zhele::Clock;

    PllClock::SelectClockSource(PllClock::Internal);
    PllClock::SetDivider(4);
    PllClock::SetMultiplier(84);
    PllClock::SetSystemOutputDivider(PllClock::SystemOutputDivider::Div4);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
}

void ConfigureTimer()
{
    DelayTimer::Enable();
    DelayTimer::SetPrescaler(0);
    DelayTimer::SetPeriod(10000); // Timer will never count to period

    DelayTimer::SlaveMode::SelectTrigger(DelayTimer::SlaveMode::Trigger::FilteredTimerInput1);
    DelayTimer::SlaveMode::EnableSlaveMode(DelayTimer::SlaveMode::Mode::ResetMode);

    InputChannel::SelectPins<InputLine>();
    InputChannel::SetCapturePolarity(InputChannel::CapturePolarity::BothEdges); // Stm32f401 support both edges capture. See RM0368, 13.4.9
    InputChannel::SetCaptureMode(InputChannel::CaptureMode::Direct);
    InputChannel::Enable();

    OuputChannel::SelectPins<OutputLine>();
    OuputChannel::SetOutputPolarity(OuputChannel::OutputPolarity::ActiveLow);
    OuputChannel::SetOutputMode(OuputChannel::OutputMode::Toggle);
    OuputChannel::SetPulse(4); // So delay is about 95ns
    OuputChannel::Enable();

    DelayTimer::Start();
}