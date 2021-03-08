#include <clock.h>
#include <iopins.h>
#include <usb.h>

using namespace Zhele::Clock;
using namespace Zhele::Usb;

/// There is some problems with init buffers table.
/// Buffer location for N endpoint depends on 0...N-1 endpoints.
/// So we must initialize buffer manager with all used endpoints.
/// Next we can get extended (with all addresses and registers assign) endpoints.

using BulkEpBase = BidirectionalEndpointBase<1, EndpointType::Bulk, 64, 50>;
using EpInitializer = EndpointsInitializer<DefaultEp0, BulkEpBase>;

// Ok, lets get initialized endpoints.
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using BulkEp = EpInitializer::ExtendEndpoint<BulkEpBase>;

// Interface with one bidirectional EP (in real it will be two endpoints with one CTR handler).
using DefaultInterface = Interface<0, 0, 0, 0, 0, BulkEp>;
// Declare configuration with one interface.
using DefaultConfiguration = Configuration<0, 200, false, false, DefaultInterface>;
// Device with one configuration. Ep0 does not belong any interface. Entire device is simultaneously EP0.
using MyDevice = Device<0x0201, DeviceClass::InterfaceSpecified, 0, 0, 0, 0, 0, Ep0>;

void ConfigureClock();
void ConfigureLeds();

int main()
{
    ConfigureClock();

    Zhele::IO::Porta::Enable();
    MyDevice::Enable();

    for(;;)
    {
    }
}

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::Internal);
    PllClock::SetMultiplier(12);
    PllClock::SetDivider(2);
    ApbClock::SetPrescaler(ApbClock::Div1);
    SysClock::SelectClockSource(SysClock::Pll);
    Zhele::Clock::SysCfgCompClock::Enable();

    // Now i have problems with usb clock.
    // TODO:: add more clock source in library.
    RCC->CR2 |= RCC_CR2_HSI48ON;
    while((RCC->CR2 & RCC_CR2_HSI48RDY) == 0){}
}

template<>
void BulkEp::Handler()
{
    // Bulk CTR handler
}

extern "C" void USB_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}