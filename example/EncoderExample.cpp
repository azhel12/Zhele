#include <timer.h>
#include <drivers/encoder.h>

using Encoder = Zhele::Drivers::Encoder<Zhele::Timers::Timer2, Zhele::IO::Pa0, Zhele::IO::Pa1, 100>;

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
    void TIM2_IRQHandler()
    {
        Value = Encoder::GetValueInterrupt();
        Zhele::Timers::Timer2::ClearInterruptFlag();
    }
}