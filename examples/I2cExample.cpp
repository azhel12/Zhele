// Define target cpu frequence.
#define F_CPU 8000000

#include <i2c.h>

using namespace Zhele;

using Interface = I2c1;

// 0xD0 - it's ds107 RTC.
int main()
{
    Interface::Init();
    Interface::SelectPins<IO::Pa9, IO::Pa10>();

	// U8 operationds
	
    //Interface::WriteU8(0xD0 >> 1, 0x06, 21);
    //auto readed = Interface::ReadU8(0xD0 >> 1, 0x06);
    //if(readed.Status == I2cStatus::Success)
    //{
        //uint8_t year = readed.Value;
    //}

    uint8_t data[555];
    //Interface::Read(0xD0 >> 1, 0x06, data, 555);
    //Interface::Write(0xD0 >> 1, 0x06, data, 555);
    //Interface::EnableAsyncRead(0xD0 >> 1, 0x06, data, 555);
    //Interface::WriteAsync(0xD0 >> 1, 0x06, data, 555);

    for (;;)
    {
    }
}