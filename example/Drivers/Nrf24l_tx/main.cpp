#include <zhele/iopins.h>
#include <zhele/spi.h>
#include <zhele/drivers/nrf24l.h>

using namespace Zhele;

using Transmitter = Drivers::Nrf24l<Spi1, IO::Pa4, IO::Pa3>;

int main()
{
    uint8_t myAddress[] = { 0x00, 0x00, 0x00, 0x00, 0x01 };
    uint8_t remoteAddress[] = { 0x00, 0x00, 0x00, 0x00, 0x02 };

    Spi1::SelectPins<IO::Pa7, IO::Pa6, IO::Pa5, IO::NullPin>();
    Transmitter::Init();

    Transmitter::SetMyAddress(myAddress);
    Transmitter::SetTxAddress(remoteAddress);
    Transmitter::PowerUpTx();

    static uint8_t data[32] = "Hello from Zhele"; 
    Transmitter::Transmit(data);

    for (;;)
    {
    }
}