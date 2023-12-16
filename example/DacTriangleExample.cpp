#include <dac.h>
#include <iopins.h>
#include <timer.h>

using namespace Zhele;
using namespace Zhele::IO;
using namespace Zhele::Timers;

int main()
{
    Timer3::Enable();
    Timer3::SetPrescaler(3);
    Timer3::SetPeriod(8);
    Timer3::EnableInterrupt();
    Timer3::Start();


    Dac1Channel1::Init(DacTrigger::Software);
    Dac1Channel1::EnableTriangleWaveGeneration(Dac1Channel1::WaveAmplitude::Equal4095);
    Dac1Channel1::Enable();    

    for (;;)
    {
    }
}

extern "C"
{
    // Toogle led on timer`s tick
    void TIM3_IRQHandler()
    {
        Dac1Channel1::CauseSoftwareTrigger();
        Timer3::ClearInterruptFlag();
    }
}