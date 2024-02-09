#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/timer.h>
#include <zhele/usart.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Clock;

void ConfigureInputCapture();

using InputTimer = Timer3;
using InputCaptureRising = InputTimer::InputCapture<0>;
using InputCaptureFalling = InputTimer::InputCapture<1>;

using UsartConnection = Usart1;

int main()
{    
    ConfigureInputCapture();

    for (;;)
    {
    }
}
void ConfigureInputCapture()
{
    InputTimer::Enable();
    InputTimer::SetPrescaler(71); // 72 Mhz / 72 = 1 Mhz. 1 tick = 1 us
    InputTimer::SetPeriod(65535);

    InputTimer::SlaveMode::SelectTrigger(InputTimer::SlaveMode::Trigger::FilteredTimerInput1);
    InputTimer::SlaveMode::EnableSlaveMode(InputTimer::SlaveMode::Mode::ResetMode);

    InputCaptureRising::SelectPins<0>();
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