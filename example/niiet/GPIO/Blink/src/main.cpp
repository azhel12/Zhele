/**
 * Blinking LED.
 *   VG015 (IRIS UNO-VG015): LED Pc10 | VG5T/VG7T (NIIET-MINI): LED Pa12, active-low
 */
#include <zhele/delay.h>
#include <zhele/iopins.h>

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
    #include <system_k1921vg015.h> // SystemInit()
    using Led = Zhele::IO::Pc10;
#else
    #if defined(ZHELE_NIIET_DEVICE_K1921VG5T)
        #include <system_k1921vg5t.h>
    #else
        #include <system_k1921vg7t.h>
    #endif
    using Led = Zhele::IO::Pa12Inv; // Set() drives the pin low and lights the LED
#endif

int main()
{
    SystemInit();

    Led::Port::Enable();
    Led::SetConfiguration<Led::Port::Configuration::Out>();
    Led::SetDriverType<Led::Port::DriverType::PushPull>();

    for (;;)
    {
        Led::Toggle();
        Zhele::delay_ms<500>();
    }
}
