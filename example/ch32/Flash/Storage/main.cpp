/**
 * Flash storage — erase the last flash page, write a string into it and read it
 * back (V003 / V006 / V203 / X035).
 *
 * The LED lights up if what was read back matches what was written.
 *
 * LED pin (adjust to your board):
 *   CH32V003 / CH32V006 -> PC4
 *   CH32V203            -> PB2
 *   CH32X035            -> PB12
 */
#include <zhele/flash.h>
#include <zhele/iopins.h>
#include <zhele/delay.h>
#include <zhele/clock.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32V203)
using Led = Pb2;
#elif defined(CH32X035)
using Led = Pb12;
#else
using Led = Pc4;
#endif

static constexpr unsigned StoragePage = Flash::PageCount() - 1;

int main()
{
#if defined(CH32X035)
    Clock::AhbClock::SetPrescaler<Clock::AhbClock::Div1>();
#endif
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    const char data[] = "Some data for store into flash";
    auto storage = reinterpret_cast<const volatile char*>(Flash::PageAddress(StoragePage));

    bool success = Flash::ErasePage(StoragePage)
        && Flash::WritePage(StoragePage, data, sizeof(data), 0);

    for (unsigned i = 0; success && i < sizeof(data); ++i) {
        success = storage[i] == data[i];
    }

    for (;;) {
        Led::Set(success);
        delay_ms<500>();
    }
}
