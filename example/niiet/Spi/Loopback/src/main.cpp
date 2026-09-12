/**
 * SPI0 internal-loopback self test (K1921VG5T/VG7T; SPI is not ported to VG015).
 *
 * The PL022 can loop its transmit shift register back into the receiver
 * (CR.LBM), so this example needs no external wiring: it sends a byte pattern,
 * checks what comes back and reports the result over UART0
 * (VG5T: TX=Pb1 RX=Pb0 | VG7T: TX=Pa1 RX=Pa0).
 *
 * Remove the loopback bit and call SelectPins() to talk to a real device
 * (VG7T: SPI0 CLK = Pa4, FSS = Pa5, RX (MISO) = Pa6, TX (MOSI) = Pa7, all AF1).
 */
#include <zhele/spi.h>
#include <zhele/usart.h>

#if defined(ZHELE_NIIET_DEVICE_K1921VG5T)
    #include <system_k1921vg5t.h> // SystemInit()
#else
    #include <system_k1921vg7t.h>
#endif

using namespace Zhele;

using Spi = Spi0<>;
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
    Uart::SelectTxRxPins<IO::Pb1, IO::Pb0>();
#else
    Uart::SelectTxRxPins<IO::Pa1, IO::Pa0>();
#endif

    Spi::Init(Spi::Div16, Spi::Master);
    Spi::Regs::Get()->CR |= SPI_CR_LBM_Msk; // internal loopback, no pins needed

    bool ok = true;
    for (uint16_t value = 0; value < 256; ++value)
        ok = ok && (Spi::Send(value) == value);

    writeString(ok ? "SPI0 loopback: OK\r\n" : "SPI0 loopback: FAILED\r\n");

    for (;;)
    {
    }
}
