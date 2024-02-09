#include <zhele/iopins.h>
#include <zhele/spi.h>
#include <zhele/drivers/nrf24l.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Receiver = Nrf24l<Spi1, IO::Pa4, IO::Pa3>;

uint8_t RxBuffer[32] = {};
int main()
{
    uint8_t myAddress[] = { 0x00, 0x00, 0x00, 0x00, 0x02 };
    Spi1::SelectPins<IO::Pa7, IO::Pa6, IO::Pa5, IO::NullPin>();
    Receiver::Init();
    Receiver::SetMyAddress(myAddress);
    Receiver::PowerUpRx();

    while(!Receiver::DataReady()){}

    Receiver::GetData(RxBuffer);

    for (;;)
    {
    }
}