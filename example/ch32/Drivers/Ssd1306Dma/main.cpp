/**
 * SSD1306 128x64 OLED over I2C with DMA (V003 / V006 / V203 / X035).
 *
 * Wiring (default I2C1 mapping, no remap):
 *   CH32V003 / CH32V006 -> SCL=PC2  SDA=PC1
 *   CH32V203            -> SCL=PB6  SDA=PB7
 *   CH32X035            -> SCL=PA10 SDA=PA11
 */

#include <zhele/i2c.h>
#include <zhele/drivers/ssd1306.h>
#include <zhele/drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Drivers;

using Lcd = Ssd1306<I2c1, 128, 64>;

// WCH-Interrupt-fast on the WCH toolchain; naked + mret on generic GCC.
#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast")))
void DMA1_Channel6_IRQHandler() {
    Dma1Channel6::IrqHandler();
}
#else
extern "C" __attribute__((naked))
void DMA1_Channel6_IRQHandler() {
    Dma1Channel6::IrqHandler();
    __asm volatile ("mret");
}
#endif

int main()
{
    Dma1::Enable();

    I2c1::Init(400000); // 400 kHz fast mode
#if defined(CH32V203)
    I2c1::SelectPins<IO::Pb6, IO::Pb7>();
#elif defined(CH32X035)
    I2c1::SelectPins<IO::Pa10, IO::Pa11>();
#else
    I2c1::SelectPins<IO::Pc2, IO::Pc1>();
#endif

    if (!Lcd::Init()) {
        for (;;) {
        }
    }

    Lcd::Goto(0, 0);
    Lcd::Puts<Font5x7>("Zhele I2C DMA");
    Lcd::Goto(0, 16);
    Lcd::Puts<Font5x7>("SSD1306 OK");
    Lcd::Update();

    for (;;) {
    }
}
