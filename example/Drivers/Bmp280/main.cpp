#include <zhele/i2c.h>
#include <zhele/drivers/bmp280.h>

using namespace Zhele;
using namespace Zhele::Drivers;

#if defined (STM32G0)
using Interface = I2c1<>;
#else
using Interface = I2c1;
#endif

using BmpSensor = Bmp280<Interface>;

int main()
{
    
    Interface::Init();
    Interface::SelectPins<IO::Pb6, IO::Pb7>();

    volatile bool initResult = BmpSensor::Init();
    volatile auto temp = BmpSensor::ReadTemperature();

    volatile int a = 42;

    for (;;)
    {
    }
}