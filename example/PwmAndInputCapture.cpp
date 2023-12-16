#define HSE_VALUE 25000000
#define HSI_VALUE 16000000
#define F_CPU 84000000

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
    PllClock::SelectClockSource(PllClock::External);
    PllClock::SetDivider(25);
    PllClock::SetMultiplier(336);
    PllClock::SetSystemOutputDivider(PllClock::SystemOutputDivider::Div4);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
}

void ConfigureOutputPwm()
{
    OuputTimer::Enable();
    OuputTimer::SetPrescaler(41); // 84 Mhz / 42 = 2 Mhz
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
    InputTimer::SetPrescaler(83); // 84 Mhz / 84 = 1 Mhz. 1 tick = 1 us
    InputTimer::SetPeriod(65535); // Max period (so we can ignore cases when CCR1 > CCR2)

    InputCaptureRising::SelectPins<Pb6>();
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
    void TIM4_IRQHandler()
    {
        if (InputCaptureFalling::IsInterrupt())
        {
            volatile auto duration = InputCaptureFalling::GetValue() - InputCaptureRising::GetValue(); // duration=5 (it's correct)
            InputCaptureFalling::ClearInterruptFlag();
        }
    }
}