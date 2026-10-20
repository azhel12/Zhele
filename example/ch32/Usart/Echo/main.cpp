/**
 * UART echo (interrupt-driven), 115200 8N1, no remap.
 *   V003/V006: USART1 TX=PD5 RX=PD6 | V203: USART1 TX=PA9 RX=PA10 | X035: USART2 TX=PA2 RX=PA3
 */
#include <zhele/usart.h>
#include <zhele/iopins.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32X035)
using Uart = Usart2;
#  define UART_IRQHandler USART2_IRQHandler
#else
using Uart = Usart1;
#  define UART_IRQHandler USART1_IRQHandler
#endif

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
void UART_IRQHandler() {
    if (Uart::ReadReady())
        Uart::Write(Uart::Read()); // reading DATAR clears RXNE
}
#else
extern "C" __attribute__((naked)) void UART_IRQHandler() {
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
#elif defined(CH32X035)
    Uart::SelectTxRxPins<Pa2, Pa3>();
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
