#include <zhele/iopins.h>
#include <zhele/i2c.h>
#include <zhele/drivers/hd44780.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Drivers;

int main()
{
    I2c1::Init();
    I2c1::SelectPins<Pb6, Pb7>();

    using lcd = LcdI2c<I2c1>;

    lcd::Init();
    lcd::Clear();
    lcd::Puts("Welcome to hell!");

    for (;;)
    {
    }
}