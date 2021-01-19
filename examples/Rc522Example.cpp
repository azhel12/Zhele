#define F_CPU 8000000

#include <clock.h>
#include <iopins.h>
#include <spi.h>
#include <timer.h>
#include <usart.h>

#include <drivers/rc522.h>

#include <stdio.h>

using namespace Zhele;
using namespace Zhele::IO;

using NfcReader = Drivers::Rc522<Spi1, IO::Pa4>;
using UsartConnection = Usart1;
using CheckTimer = Timers::Timer3;

void ConfigureUsart();
void ConfigureNfcReader();
void ConfigureTimer();

uint8_t buffer[8];

int main()
{	
    ConfigureUsart();
    ConfigureNfcReader();
    ConfigureTimer();
    for (;;)
    {
    }
}

void ConfigureUsart()
{
    UsartConnection::Init(9600);
    UsartConnection::SelectTxRxPins<Pa9, Pa10>();
    UsartConnection::Write("Hello!\r\n", 8);
}

void ConfigureNfcReader()
{
    Spi1::Init(Spi1::ClockDivider::Fast);
    // Disable JTAG for use Pa15/Pb3/Pb4 on Stm32F1
    //SwjRemap::Set(2);

    Spi1::SelectPins<Pa7, Pa6, Pa5, NullPin>();
    NfcReader::Init();
}

void ConfigureTimer()
{
    CheckTimer::Enable();
    CheckTimer::SetPrescaler(3999);
    CheckTimer::SetPeriod(1999);
    CheckTimer::EnableInterrupt();
    CheckTimer::Start();
}

extern "C"
{
    void TIM3_IRQHandler()
    {
        static char printbuffer[32];
        CheckTimer::ClearInterruptFlag();

        volatile auto status = NfcReader::Check(buffer);
        if(status == NfcReader::Status::Success)
        {
            // Very heavy method: with -Os optimization it takes ~2300 bytes (from ~4800 total!!!)
            // So, startup + usart + timer + Rfid (+Spi) takes ~2500. "Great" stdlib=)
            sprintf(printbuffer, "RFID: 0x%02x 0x%02x 0x%02x 0x%02x\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
            UsartConnection::Write(printbuffer, 27);
        }
        if(status == NfcReader::Status::NoTagError)
        {
            UsartConnection::Write("No tag\r\n", 8);
        }
    }
}
