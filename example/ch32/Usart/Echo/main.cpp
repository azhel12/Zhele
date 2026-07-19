/**
 * UART echo (interrupt-driven), USART1, 115200 8N1.
 *   V003/V006: TX=PD5 RX=PD6 | V203: TX=PA9 RX=PA10  (no remap)
 */
#include <zhele/usart.h>
#include <zhele/iopins.h>

using namespace Zhele;
using namespace Zhele::IO;

using Uart = Usart1;

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
void USART1_IRQHandler() {
    if (Uart::ReadReady())
        Uart::Write(Uart::Read()); // reading DATAR clears RXNE
}
#else
extern "C" __attribute__((naked)) void USART1_IRQHandler() {
    if (Uart::ReadReady())
        Uart::Write(Uart::Read());
    __asm volatile ("mret");
}
#endif

int main()
{
    Uart::Init(115200);
#if defined(CH32V203)
    Uart::SelectTxRxPins<Pa9, Pa10>();
#else
    Uart::SelectTxRxPins<Pd5, Pd6>();
#endif

    Uart::EnableInterrupt(Uart::InterruptFlags::RxNotEmptyInt);

    const char hello[] = "Zhele UART echo\r\n";
    Uart::Write(hello, sizeof(hello) - 1);

    for (;;)
    {
    }
}
