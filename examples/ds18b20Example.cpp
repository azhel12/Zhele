// For stm32f401 HSI_VALUE is 16Mhz. It's important for USART configuration.
#define HSI_VALUE 16000000
#define F_CPU 16000000

// For other my controllers (f030, f072, f103)
//#define F_CPU 8000000

#include <iopins.h>
#include <usart.h>
#include <one_wire.h>
#include <drivers/ds18b20.h>

using namespace Zhele;

using OneWireBus = OneWire<Usart1, IO::Pa9>;
using TempSensor = Drivers::Ds18b20<OneWireBus>;

int main()
{
    TempSensor::Init();
    TempSensor::Start();
    
    while(!TempSensor::AllDone());

    auto convert = TempSensor::Read();
    if(convert.Success)
    {
        // voaltile for debug (anti-optimization)
        volatile float temp = convert.Temperature;
        volatile int a = 42;
    }
    else
    {
        volatile auto error = convert.Error;
        volatile int a = 42;
    }

    
    for (;;)
    {
    }
}