// Attention please!!! This example consists from 2 parts: Receiver and Transmitter.
// Run them on different mcus.

// Define target cpu frequence.
#define F_CPU 8000000

#include <iopins.h>
#include <i2c.h>
#include <spi.h>
#include <drivers/nrf24l.h>
#include <drivers/fonts.h>
#include <drivers/ssd1306.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Receiver = Nrf24l<Spi1, IO::Pa4, IO::Pa3>;
using Lcd = Ssd1306<I2c1>;

uint8_t RxBuffer[32] = {};
int main()
{
    I2c1::Init(400000);
    I2c1::SelectPins<IO::Pb6, IO::Pb7>();
    Lcd::Init();
    Lcd::Puts<TimesNewRoman13>("Waiting...");
    Lcd::Update();

    uint8_t myAddress[] = { 0x00, 0x00, 0x00, 0x00, 0x02 };
    Spi1::SelectPins<IO::Pa7, IO::Pa6, IO::Pa5, IO::NullPin>();
    Receiver::Init();
    Receiver::SetMyAddress(myAddress);
    Receiver::PowerUpRx();

    while(!Receiver::DataReady()){}

    Receiver::GetData(RxBuffer);
    Lcd::Fill(Lcd::Pixel::Off);
    Lcd::Goto(0, 0);
    Lcd::Puts<TimesNewRoman13>(reinterpret_cast<const char*>(RxBuffer));
    Lcd::Update();

    for (;;)
    {
    }
}

// RAM: 304; Flash: 4460