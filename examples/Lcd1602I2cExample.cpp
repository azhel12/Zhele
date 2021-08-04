#define F_CPU 8000000

#include <iopins.h>
#include <i2c.h>
#include <drivers/hd44780.h>

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

// RAM: 28; Flash: 476