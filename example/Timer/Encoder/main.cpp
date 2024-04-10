#include <zhele/timer.h>
#include <zhele/drivers/encoder.h>

using Encoder = Zhele::Drivers::Encoder<Zhele::Timers::Timer3>;

uint16_t Value;

int main()
{
    Encoder::Init();
    Encoder::EnableInterrupt();

    for(;;)
    {
    }
}

extern "C"
{
    void TIM3_IRQHandler()
    {
        Value = Encoder::GetValueInterrupt();
        Zhele::Timers::Timer3::ClearInterruptFlag();
    }
}