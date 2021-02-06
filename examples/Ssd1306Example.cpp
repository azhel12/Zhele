// Define target cpu frequence.
#define F_CPU 8000000

#include <i2c.h>
#include <drivers/ssd1306.h>
#include <drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Lcd = Ssd1306<I2c1>;

int main()
{
    I2c1::Init(400000);
    I2c1::SelectPins<IO::Pa9, IO::Pa10>();

    Lcd::Init();
    Lcd::Puts<TimesNewRoman13>("Abcdefghijklmnopqrstu");
    Lcd::Update();
    
    for (;;)
    {
    }
}