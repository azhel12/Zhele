#include <zhele/iopins.h>
#include <zhele/usart.h>
#include <zhele/one_wire.h>
#include <zhele/drivers/ds18b20.h>

using namespace Zhele;

using OneWireBus = OneWire<Usart1, IO::Pa9>;
using TempSensor = Drivers::Ds18b20<OneWireBus>;

int main()
{
    TempSensor::Init();
    TempSensor::Start();
    
    while(!TempSensor::AllDone());

    auto convert = TempSensor::Read();
    if(convert.Success) {
        // voaltile for debug (anti-optimization)
        volatile float temp = convert.Temperature;
        volatile int a = 42;
    }
    else {
        volatile auto error = convert.Error;
        volatile int a = 42;
    }
    
    for (;;)
    {
    }
}