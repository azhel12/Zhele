#include <zhele/i2c.h>
#include <zhele/drivers/aht10.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Sensor = Aht10<I2c1>;

int main()
{
    
    I2c1::Init();
    I2c1::SelectPins<IO::Pb8, IO::Pb9>();

    volatile bool initResult = Sensor::Init();
    volatile auto temp = Sensor::ReadTemperatureAndHumidity();

    volatile int a = 42;

    for (;;)
    {
    }
}