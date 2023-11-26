// Define target cpu frequence.
#define F_CPU 8000000

#include <iopins.h>
#include <timer.h>

using namespace Zhele::IO;
using namespace Zhele::Timers;

using PwmChannel = Timer3::PWMGeneration<1>;
int main()
{
    Pa7::Port::Enable();

    Timer3::Enable();
	Timer3::SetPrescaler(799);
	Timer3::SetPeriod(100);
    
    // Select pin (A7)
    PwmChannel::SelectPins<Pa7>();
    // Enable compare
    PwmChannel::Enable();
    // Set output mode
	PwmChannel::SetOutputMode(PwmChannel::OutputMode::PWM1);
    // Set PWM level
    PwmChannel::SetPulse(3);
	// Start timer
	Timer3::Start();
    
    for (;;)
    {
    }
}