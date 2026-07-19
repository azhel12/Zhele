/**
 * SPI1 loopback self-test over DMA — jumper MOSI to MISO. A full-duplex DMA
 * transfer (TX=DMA1 ch3, RX=DMA1 ch2) sends a byte pattern; the callback checks
 * the received buffer equals the sent one. LED shows the result:
 *   solid ON  = loopback OK      blinking = mismatch
 *
 *   V003/V006: SCK=PC5 MISO=PC7 MOSI=PC6  (jumper PC6<->PC7), LED=PC4
 *   V203:      SCK=PA5 MISO=PA6 MOSI=PA7  (jumper PA7<->PA6), LED=PB2
 */
#include <zhele/spi.h>
#include <zhele/iopins.h>
#include <zhele/dma.h>
#include <zhele/delay.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32V203)
using Led  = Pb2;
using Sck  = Pa5;
using Miso = Pa6;
using Mosi = Pa7;
#else
using Led  = Pc4;
using Sck  = Pc5;
using Miso = Pc7;
using Mosi = Pc6;
#endif

static const uint8_t txBuf[] = {0xA5, 0x3C, 0xFF, 0x00, 0x5A};
static uint8_t rxBuf[sizeof(txBuf)];
static volatile bool g_done = false;
static volatile bool g_ok   = false;

void OnTransfer(void* /*data*/, unsigned size, bool success)
{
    if (success) {
        g_ok = true;
        for (unsigned i = 0; i < size; ++i)
            if (rxBuf[i] != txBuf[i])
                g_ok = false;
    }
    g_done = true;
}

int main()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();

    Dma1::Enable();

    Spi1::Init(Spi1::Medium, Spi1::Master);
    Spi1::SelectPins<Mosi, Miso, Sck>();

    Spi1::SendAsync(txBuf, rxBuf, sizeof(txBuf), OnTransfer);

    while (!g_done) {
    }

    if (g_ok) {
        Led::Set(); // solid on = loopback OK
        for (;;) {
        }
    }

    for (;;) { // fast blink = mismatch
        Led::Toggle();
        delay_ms<150>();
    }
}

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA1_Channel2_IRQHandler() { Spi1::DmaRx::IrqHandler(); }
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA1_Channel3_IRQHandler() { Spi1::DmaTx::IrqHandler(); }
#else
extern "C" __attribute__((naked)) void DMA1_Channel2_IRQHandler() { Spi1::DmaRx::IrqHandler(); __asm volatile ("mret"); }
extern "C" __attribute__((naked)) void DMA1_Channel3_IRQHandler() { Spi1::DmaTx::IrqHandler(); __asm volatile ("mret"); }
#endif
