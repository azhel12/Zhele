#include <zhele/i2c.h>

using namespace Zhele;


#if defined (STM32G0)
#include <zhele/dma.h>
#include <zhele/dmamux.h>
using Interface = I2c1<Dma1Channel1, Dma1Channel2>;
#else
using Interface = I2c1;
#endif

// 0xD0 - it's ds107 RTC.
int main()
{
    Interface::Init();
    Interface::SelectPins<IO::Pb6, IO::Pb7>();

    // U8 operationds
    Interface::WriteU8(0xD0 >> 1, 0x06, 21);
    auto readed = Interface::ReadU8(0xD0 >> 1, 0x06);
    if(readed.Status == I2cStatus::Success)
    {
        uint8_t year = readed.Value;
    }

    uint8_t data[555];
    Interface::Read(0xD0 >> 1, 0x06, data, 555);
    Interface::Write(0xD0 >> 1, 0x06, data, 555);


    // Or use stream write with DMA
#if defined (STM32G0)
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::I2c1Tx);
    DmaMux1Channel2::SelectRequestInput(DmaMux1::RequestInput::I2c1Rx);
#endif
    Interface::EnableAsyncRead(0xD0 >> 1, 0x06, data, 555);
    Interface::WriteAsync(0xD0 >> 1, 0x06, data, 555);

    for (;;)
    {
    }
}