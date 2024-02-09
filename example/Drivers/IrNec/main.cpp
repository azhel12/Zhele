#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/timer.h>
#include <zhele/drivers/ir.h>

using namespace Zhele;
using namespace Zhele::Drivers;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Clock;

using Receiver = IrReceiver<Timer3, Pb6, NecDecoder>;

int main()
{    
    Receiver::Init();
    NecDecoder::SetCallback([](NecDecoder::Command command) {
        // Do smth
    });

    for (;;)
    {
    }
}

extern "C"
{
    void TIM3_IRQHandler() {
        Receiver::IRQHandler();
    }
}