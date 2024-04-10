#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/timer.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Clock;

void ConfigureInputCapture();

using InputTimer = Timer3;
using InputCaptureRising = InputTimer::InputCapture<0>;
using InputCaptureFalling = InputTimer::InputCapture<1>;

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
    InputTimer::SetPrescaler(83); // 84 Mhz / 84 = 1 Mhz. 1 tick = 1 us
    InputTimer::SetPeriod(65535); // Max period (so we can ignore cases when CCR1 > CCR2)

    InputCaptureRising::SelectPins<0>();
    InputCaptureRising::SetCapturePolarity(InputCaptureRising::CapturePolarity::RisingEdge);
    InputCaptureRising::SetCaptureMode(InputCaptureRising::CaptureMode::Direct);
    InputCaptureRising::Enable();

    InputCaptureFalling::SetCapturePolarity(InputCaptureFalling::CapturePolarity::FallingEdge);
    InputCaptureFalling::SetCaptureMode(InputCaptureFalling::CaptureMode::Indirect);
    InputCaptureFalling::EnableInterrupt();
    InputCaptureFalling::Enable();

    InputTimer::Start();
}

extern "C"
{
    void TIM3_IRQHandler()
    {
        if (InputCaptureFalling::IsInterrupt())
        {
            volatile auto duration = InputCaptureFalling::GetValue() - InputCaptureRising::GetValue(); // duration=5 (it's correct)
            InputCaptureFalling::ClearInterruptFlag();
        }
    }
}