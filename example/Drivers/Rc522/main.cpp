#include <zhele/iopins.h>
#include <zhele/spi.h>
#include <zhele/timer.h>

#include <zhele/drivers/rc522.h>

#include <stdio.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined (STM32G0)
using Interface = Spi1<>;
#else
using Interface = Spi1;
#endif

using NfcReader = Drivers::Rc522<Interface, IO::Pa4>;
using CheckTimer = Timers::Timer3;

void ConfigureNfcReader();
void ConfigureTimer();

uint8_t buffer[8];

int main()
{	
    ConfigureNfcReader();
    ConfigureTimer();

    for (;;)
    {
    }
}

void ConfigureNfcReader()
{
    Interface::Init(Interface::ClockDivider::Fast);
    // Disable JTAG for use Pa15/Pb3/Pb4 on Stm32F1
    //SwjRemap::Set(2);

    Interface::SelectPins<Pa7, Pa6, Pa5, NullPin>();
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
        CheckTimer::ClearInterruptFlag();

        volatile auto status = NfcReader::Check(buffer);
        if(status == NfcReader::Status::Success)
        {
            // Do smth
        }
    }
}
