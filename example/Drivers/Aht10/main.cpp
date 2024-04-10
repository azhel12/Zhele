#include <zhele/i2c.h>
#include <zhele/drivers/aht10.h>

using namespace Zhele;
using namespace Zhele::Drivers;


#if defined (STM32G0)
using Interface = I2c1<>;
#else
using Interface = I2c1;
#endif

using Sensor = Aht10<Interface>;

int main()
{
    
    Interface::Init();
    Interface::SelectPins<IO::Pb8, IO::Pb9>();

    volatile bool initResult = Sensor::Init();
    volatile auto temp = Sensor::ReadTemperatureAndHumidity();

    volatile int a = 42;

    for (;;)
    {
    }
}