#include <zhele/i2c.h>
#include <zhele/drivers/ssd1306.h>
#include <zhele/drivers/fonts.h>

using namespace Zhele;
using namespace Zhele::Drivers;

#if defined (STM32G0)
#include <zhele/dma.h>
#include <zhele/dmamux.h>
using Interface = I2c1<Dma1Channel1, Dma1Channel2>;
#else
using Interface = I2c1;
#endif

using Lcd = Ssd1306<Interface>;

int main()
{
    Interface::Init(400000);
    Interface::SelectPins<IO::Pb6, IO::Pb7>();

    // ssd1306 requires DMA
#if defined (STM32G0)
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::I2c1Tx);
    DmaMux1Channel2::SelectRequestInput(DmaMux1::RequestInput::I2c1Rx);
#endif

    Lcd::Init();
    Lcd::Puts<TimesNewRoman13>("Abcdefghijklmnopqrstu");
    Lcd::Update();
    
    for (;;)
    {
    }
}