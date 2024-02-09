#include <zhele/clock.h>

using namespace Zhele::Clock;

void ConfigureClock();

int main()
{
    ConfigureClock();

    for (;;)
    {
    }
}

#if defined (STM32F0) // F072RB
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource(PllClock::External);
    // Set PLL multiplier
    PllClock::SetMultiplier(6);
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    // Try select PLL as sysclock source
    SysClock::SelectClockSource(SysClock::Pll);
}
#elif defined (STM32F1) // F103C8
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource(PllClock::External);
    // Set PLL multiplier
    PllClock::SetMultiplier(9);
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    // Try select PLL as sysclock source
    SysClock::SelectClockSource(SysClock::Pll);
}
#elif defined (STM32F4) // F401CC
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource(PllClock::External);
    // Set PLLM value
    PllClock::SetDivider(25);
    // Set PllN value
    PllClock::SetMultiplier(336);
    // Set PLLP
    PllClock::SetSystemOutputDivider(PllClock::SystemOutputDivider::Div4);
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    // Try select PLL as sysclock source
    SysClock::SelectClockSource(SysClock::Pll);
}
#else
    #error "No example"
#endif