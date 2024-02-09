#include <zhele/i2c.h>
#include <zhele/drivers/bmp280.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using BmpSensor = Bmp280<I2c1>;

int main()
{
    
    I2c1::Init();
    I2c1::SelectPins<IO::Pb6, IO::Pb7>();

    volatile bool initResult = BmpSensor::Init();
    volatile auto temp = BmpSensor::ReadTemperature();

    volatile int a = 42;

    for (;;)
    {
    }
}