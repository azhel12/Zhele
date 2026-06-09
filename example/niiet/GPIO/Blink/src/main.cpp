#include <zhele/iopins.h>

#include <system_k1921vg015.h> // SystemInit()

// Tested on IRIS=UNO-VG015
using Led = Zhele::IO::Pc10;

static void delay()
{
    for (unsigned i = 2'000'000u; i != 0u; --i)
        __asm volatile ("nop");
}

int main()
{
    SystemInit();

    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    for (;;)
    {
        Led::Toggle();
        delay();
    }
}
