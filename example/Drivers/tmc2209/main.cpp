#include <zhele/iopins.h>
#include <zhele/usart.h>
#include <zhele/drivers/tmc2209.h>

using tmc_usart = Zhele::Usart2<>;
using tmc_usart_tx_pin = Zhele::IO::Pa2;
using tmc = Zhele::Drivers::tmc2209<tmc_usart, tmc_usart_tx_pin>;

int main()
{
    tmc::init(9600);
    
    tmc::setRunCurrent(50);
    tmc::setHoldCurrent(50);
    tmc::setMicrostepsPerStep(256);
    
    tmc::enableAutomaticGradientAdaptation();
    tmc::enableAutomaticCurrentScaling();
    tmc::enableStealthChop();
    tmc::enableCoolStep();
    tmc::enable();
    tmc::moveAtVelocity(350000);

    for (;;)
    {
    }
}