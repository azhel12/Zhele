#define F_CPU 8000000

#include <delay.h>
#include <iopins.h>
#include <spi.h>

using namespace Zhele;
using namespace Zhele::IO;

using SpiInterface = Spi1;

// It's hard to write spi example :)
// I'll write data to nothing, you can check it with logic analyzer.
// See drivers examples.
int main()
{
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
    uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
    SpiInterface::WriteAsync(data, 4);

    for (;;)
    {
    }
}
