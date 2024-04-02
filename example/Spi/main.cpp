#include <zhele/dmamux.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>
#include <zhele/spi.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined (STM32G0)
using SpiInterface = Spi1<Dma1Channel1, Dma1Channel2>;
#else
using SpiInterface = Spi1;
#endif
// It's hard to write spi example :)
// I'll write data to nothing, you can check it with logic analyzer.
// See drivers examples.
int main()
{
	// Disable JTAG for use Pa15/Pb3/Pb4 on Stm32F1, if you want to use second pin set.
    //Clock::AfioClock::Enable();
    //SwjRemap::Set(2);

    SpiInterface::Init();
    SpiInterface::SelectPins<Pa7, Pa6, Pa5, Pa4>();
    // Or
    SpiInterface::SelectPins<0, 0, 0, 0>();
    // Or
    SpiInterface::SelectPins(0, 0, 0, 0);

    SpiInterface::Write(0xde);
    SpiInterface::Write(0xad);
    SpiInterface::Write(0xbe);
    SpiInterface::Write(0xef);

    
    // Or use stream write with DMA
#if defined (STM32G0)
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::Spi1Tx);
    DmaMux1Channel2::SelectRequestInput(DmaMux1::RequestInput::Spi1Rx);
#endif
    uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
    SpiInterface::WriteAsync(data, 4);

    for (;;)
    {
    }
}