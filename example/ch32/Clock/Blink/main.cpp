/**
 * TIM2 blink whose rate steps as SYSCLK is switched HSI <-> PLL at runtime
 * (both target 48 MHz PLL; visible step confirms the switch took effect).
 *   LED: PC4 (V003/V006) or PB2 (V203)
 */
#include <zhele/iopins.h>
#include <zhele/timer.h>
#include <zhele/clock.h>

using namespace Zhele::IO;
using namespace Zhele::Clock;
using namespace Zhele::Timers;

#if defined(CH32V203)
using Led = Pb2;
#else
using Led = Pc4;
#endif

static volatile uint32_t g_blinks = 0;

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
void TIM2_IRQHandler() {
    Timer2::ClearInterruptFlag();
    Led::Toggle();
    ++g_blinks;
}
#else
extern "C" __attribute__((naked)) void TIM2_IRQHandler() {
    Timer2::ClearInterruptFlag();
    Led::Toggle();
    ++g_blinks;
    __asm volatile ("mret");
}
#endif

// Spin until the ISR has toggled the LED n more times (clock-rate independent).
static void HoldBlinks(uint32_t n)
{
    const uint32_t start = g_blinks;
    while (g_blinks - start < n) {
    }
}

int main()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    // Park SYSCLK on HSI before touching the PLL.
    SysClock::SelectClockSource<SysClock::Internal>();

    // Buses to /1 so TIM2 sees SYSCLK directly (V0 has no APB prescaler).
    AhbClock::SetPrescaler<AhbClock::Div1>();
#if defined(CH32V203)
    Apb1Clock::SetPrescaler<Apb1Clock::Div1>();
    Apb2Clock::SetPrescaler<Apb2Clock::Div1>();
#endif

    // Configure the PLL for a 48 MHz output (off while we reconfigure it).
    PllClock::Disable();
    PllClock::SelectClockSource<PllClock::Internal>();
#if defined(CH32V203)
    PllClock::SetMultiplier<12>();  // HSI/2 (4 MHz) x12 = 48 MHz
#else
    PllClock::SetMultiplier<2>();   // V003/V006: fixed x2 -> HSI x2 = 48 MHz
#endif

    // TIM2 from HCLK: 1 kHz tick on HSI, / 500 -> 2 Hz overflow -> 1 Hz toggle.
    Timer2::Enable();
    Timer2::SetPrescaler(static_cast<uint16_t>(HSI_VALUE / 1000 - 1));
    Timer2::SetPeriod(500 - 1);
    Timer2::Start();
    Timer2::ClearInterruptFlag();
    Timer2::EnableInterrupt();

    for (;;) {
        SysClock::SelectClockSource<SysClock::Pll>();       // 48 MHz -> fast blink
        HoldBlinks(6);
        SysClock::SelectClockSource<SysClock::Internal>();  // HSI -> slow blink
        HoldBlinks(6);
    }
}
