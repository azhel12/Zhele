#include <zhele/i2c.h>

using namespace Zhele;

#if defined (STM32G0)
    using Interface = I2c1NoDma;
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

    if constexpr (!std::is_same_v<Interface::DmaRx, void>) {
        Interface::EnableAsyncRead(0xD0 >> 1, 0x06, data, 555);
    }

    if constexpr (!std::is_same_v<Interface::DmaTx, void>) {
        Interface::WriteAsync(0xD0 >> 1, 0x06, data, 555);
    }

    for (;;)
    {
    }
}