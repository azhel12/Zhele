// Define target cpu frequence.
#define F_CPU 8000000

#include <i2c.h>
#include <drivers/bmp280.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using BmpSendor = Bmp280<I2c1>;

int main()
{
    
    I2c1::Init();
    I2c1::SelectPins<IO::Pb6, IO::Pb7>();

    volatile bool initResult = BmpSendor::Init();
    volatile auto temp = BmpSendor::ReadTemperature();

    volatile int a = 42;

    for (;;)
    {
    }
}