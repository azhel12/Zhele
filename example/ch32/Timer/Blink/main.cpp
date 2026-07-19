/**
 * TIM2 update-interrupt blink (~1 Hz).
 *   LED: PC4 (V003/V006) or PB2 (V203)
 */
#include <zhele/iopins.h>
#include <zhele/timer.h>

using namespace Zhele::IO;
using namespace Zhele::Timers;

#if defined(CH32V203)
using Led = Pb2;
#else
using Led = Pc4;
#endif

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
void TIM2_IRQHandler() {
    Timer2::ClearInterruptFlag();
    Led::Toggle();
}
#else
extern "C" __attribute__((naked)) void TIM2_IRQHandler() {
    Timer2::ClearInterruptFlag();
    Led::Toggle();
    __asm volatile ("mret");
}
#endif

int main()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    Timer2::Enable();

    // 10 kHz tick, overflow every 5000 ticks -> 2 Hz update -> 1 Hz LED toggle.
    constexpr unsigned Tick = 10'000;
    Timer2::SetPrescaler(static_cast<uint16_t>(Timer2::GetClockFreq() / Tick - 1));
    Timer2::SetPeriod(static_cast<uint16_t>(Tick / 2 - 1));

    Timer2::Start();
    Timer2::ClearInterruptFlag();
    Timer2::EnableInterrupt();

    for (;;) {
    }
}
