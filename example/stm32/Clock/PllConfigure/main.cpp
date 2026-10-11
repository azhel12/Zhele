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
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    // Set PLL multiplier
    PllClock::SetMultiplier<6>();
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#elif defined (STM32F1) // F103C8
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    // Set PLL multiplier
    PllClock::SetMultiplier<9>();
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#elif defined (STM32F4) // F401CC
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource<PllClock::External>();
    // Set PLLM value
    PllClock::SetDivider<25>();
    // Set PllN value
    PllClock::SetMultiplier<336>();
    // Set PLLP
    PllClock::SetSystemOutputDivider<4>();
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#elif defined (STM32G0) // G030F6
void ConfigureClock()
{
    // Select HSE as PLL source	
    PllClock::SelectClockSource<PllClock::Internal>();
    // Set PLLM value
    PllClock::SetDivider<2>();
    // Set PllN value
    PllClock::SetMultiplier<32>();
    // Set PLLP
    PllClock::SetSystemOutputDivider<4>();
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#elif defined (STM32H5) // H503CB
void ConfigureClock()
{
    // 250 MHz needs the highest core voltage scale; raise it before the frequency
    SetVoltageScale(VoltageScale::Scale0);

    // HSI (64 MHz) / PLL1M = 8 -> 8 MHz reference
    PllClock::SelectClockSource<PllClock::Internal>();
    // SetDivider also programs PLL1RGE from the resulting reference frequency
    PllClock::SetDivider<8>();
    // 8 MHz * 62 = 496 MHz VCO (wide range is 128..560 MHz)
    PllClock::SetMultiplier<62>();
    // pll1_p_ck = 496 / 2 = 248 MHz
    PllClock::SetSystemOutputDivider<2>();
    // Set Apb1 prescaler
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#elif defined (STM32G4) // G474CE, HSE 8 MHz
void ConfigureClock()
{
    // HSE (8 MHz) / PLLM = 2 -> 4 MHz reference
    PllClock::SelectClockSource<PllClock::External>();
    PllClock::SetDivider<2>();
    // 4 MHz * 85 = 340 MHz VCO
    PllClock::SetMultiplier<85>();
    // PLLR: 340 / 2 = 170 MHz. Boost mode, flash latency and the AHB /2 step
    // RM0440 requires above 80 MHz are handled by SysClock::SelectClockSource()
    PllClock::SetSystemOutputDivider<2>();
    // Try select PLL as sysclock source
    SysClock::SelectClockSource<SysClock::Pll>();
}
#else
    #error "No example"
#endif