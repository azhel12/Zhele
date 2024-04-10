#include <zhele/iopins.h>
#include <zhele/i2c.h>
#include <zhele/drivers/hd44780.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Drivers;

#if defined (STM32G0)
using Interface = I2c1<Dma1Channel1, Dma1Channel2>;
#else
using Interface = I2c1;
#endif

int main()
{
    Interface::Init();
    Interface::SelectPins<Pb6, Pb7>();

    using lcd = LcdI2c<Interface>;

    lcd::Init();
    lcd::Clear();
    lcd::Puts("Welcome to hell!");

    for (;;)
    {
    }
}