#include <zhele/i2c.h>
#include <zhele/drivers/ssd1306.h>
#include <zhele/drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Lcd = Ssd1306<I2c1>;

int main()
{
    I2c1::Init(400000);
    I2c1::SelectPins<IO::Pb6, IO::Pb7>();

    Lcd::Init();
    Lcd::Puts<TimesNewRoman13>("Abcdefghijklmnopqrstu");
    Lcd::Update();
    
    for (;;)
    {
    }
}