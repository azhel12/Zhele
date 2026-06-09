#include <zhele/iopins.h>
#include <zhele/pinlist.h>

#include <system_k1921vg015.h> // SystemInit()

// Tested on IRIS=UNO-VG015
// 4-bit virtual port on PC10..PC13 (LEDs show binary counter 0..15)
using LedPort = Zhele::IO::PinList<
    Zhele::IO::Pc10,
    Zhele::IO::Pc11,
    Zhele::IO::Pc12,
    Zhele::IO::Pc13>;

static void delay()
{
    for (unsigned i = 2'000'000u; i != 0u; --i)
        __asm volatile ("nop");
}

int main()
{
    SystemInit();

    LedPort::Enable();
    LedPort::SetConfiguration<LedPort::Configuration::Out>();
    LedPort::SetDriverType<LedPort::DriverType::PushPull>();

    for (;;)
    {
        for (uint8_t n = 0; n < 16; ++n)
        {
            LedPort::Write(n);
            delay();
        }
    }
}
