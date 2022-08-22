#define F_CPU 72000000

#include <clock.h>
#include <iopins.h>
#include <timer.h>
#include <usart.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Clock;

void ConfigureClock();
void ConfigureOutputPwm();
void ConfigureInputCapture();

using OuputTimer = Timer3;
using OuputPwm = OuputTimer::PWMGeneration<1>; 

using InputTimer = Timer4;

using InputCaptureRising = InputTimer::InputCapture<0>;
using InputCaptureFalling = InputTimer::InputCapture<1>;

using UsartConnection = Usart1;

int main()
{    
    ConfigureClock();
    
    ConfigureOutputPwm();
    ConfigureInputCapture();

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

void ConfigureOutputPwm()
{
    OuputTimer::Enable();
    OuputTimer::SetPrescaler(35); // 72 Mhz / 36 = 2 Mhz
    OuputTimer::SetPeriod(19); // 2 Mhz / 20 = 100 kHz PWN frequence. Period = 10 us.

    OuputPwm::SelectPins<Pa7>();
    OuputPwm::Enable();
    OuputPwm::SetOutputMode(OuputPwm::OutputMode::PWM1);
    OuputPwm::SetPulse(10); // 50% pulse. High level duration = 5 us.
    
    OuputTimer::Start();
}

void ConfigureInputCapture()
{
    InputTimer::Enable();
    InputTimer::SetPrescaler(71); // 72 Mhz / 72 = 1 Mhz. 1 tick = 1 us
    InputTimer::SetPeriod(65535);

    InputTimer::SlaveMode::SelectTrigger(InputTimer::SlaveMode::Trigger::FilteredTimerInput1);
    InputTimer::SlaveMode::EnableSlaveMode(InputTimer::SlaveMode::Mode::ResetMode);

    InputCaptureRising::SelectPins<Pb6>();
    InputCaptureRising::SetCapturePolarity(InputCaptureRising::CapturePolarity::RisingEdge);
    InputCaptureRising::SetCaptureMode(InputCaptureRising::CaptureMode::Direct);
    InputCaptureRising::Enable();

    InputCaptureFalling::SetCapturePolarity(InputCaptureFalling::CapturePolarity::FallingEdge);
    InputCaptureFalling::SetCaptureMode(InputCaptureFalling::CaptureMode::Indirect);
    InputCaptureFalling::EnableInterrupt();
    InputCaptureFalling::Enable();

    // Also we could use only first channel and measure full period (if we know that signal is 50% PWM then duration = (CCR1 + 1) / 2)

    InputTimer::Start();
}

extern "C"
{
    void TIM4_IRQHandler()
    {
        if (InputCaptureFalling::IsInterrupt())
        {
            volatile auto duration = InputCaptureFalling::GetValue() + 1; // duration=5 (it's correct)
            InputCaptureFalling::ClearInterruptFlag();
        }
    }
}