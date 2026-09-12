/**
 * UART0 echo, polled, 115200 8N1.
 *   VG015, VG7T: TX=Pa1 RX=Pa0 | VG5T: TX=Pb1 RX=Pb0 (the NIIET-MINI USB-UART bridge)
 */
#include <zhele/usart.h>

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
    #include <system_k1921vg015.h> // SystemInit()
#elif defined(ZHELE_NIIET_DEVICE_K1921VG5T)
    #include <system_k1921vg5t.h>
#else
    #include <system_k1921vg7t.h>
#endif

using namespace Zhele;
using namespace Zhele::IO;

using Uart = Usart0<>;

static void writeString(const char* s)
{
    while (*s)
        Uart::Write(static_cast<uint8_t>(*s++));
}

int main()
{
    SystemInit();

    Uart::Init<115200>();
#if defined(ZHELE_NIIET_DEVICE_K1921VG5T)
    Uart::SelectTxRxPins<Pb1, Pb0>();
#else
    Uart::SelectTxRxPins<Pa1, Pa0>();
#endif

    writeString("NIIET UART0 echo ready\r\n");

    for (;;)
    {
        if (Uart::ReadReady())
            Uart::Write(Uart::Read());
    }
}
