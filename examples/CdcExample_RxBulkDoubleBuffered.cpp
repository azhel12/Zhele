#include <clock.h>
#include <exti.h>
#include <iopins.h>
#include <pinlist.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

using CdcCommEndpointBase = InEndpointBase<1, EndpointType::Interrupt, 8, 0xff>;
using CdcDataEndpointBase = BulkDoubleBufferedEndpointBase<2, EndpointDirection::Out, 64>;
using CdcDataEndpointBaseIn = BulkDoubleBufferedEndpointBase<3, EndpointDirection::In, 64>;

using EpInitializer = EndpointsInitializer<DefaultEp0, CdcCommEndpointBase, CdcDataEndpointBase, CdcDataEndpointBaseIn>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using CdcCommEndpoint = EpInitializer::ExtendEndpoint<CdcCommEndpointBase>;
using CdcDataEndpoint = EpInitializer::ExtendEndpoint<CdcDataEndpointBase>;
using CdcDataEndpointIn = EpInitializer::ExtendEndpoint<CdcDataEndpointBaseIn>;

using CdcComm = DefaultCdcCommInterface<0, Ep0, CdcCommEndpoint>;
using CdcData = CdcDataInterface<1, 0, 0, 0, Ep0, CdcDataEndpoint, CdcDataEndpointIn>;

using Config = Configuration<0, 250, false, false, CdcComm, CdcData>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Comm, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc7;

void ConfigureClock();
void ConfigureLeds();

int main()

{
    ConfigureClock();
    ConfigureLeds();
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

    Zhele::Clock::Hsi48Clock::Enable();
    Zhele::Clock::SysCfgCompClock::Enable();
}

void ConfigureLeds()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();
    Led::Set();
}

template<>
void CdcDataEndpoint::HandleRx()
{
    uint8_t* data;
    if (CdcDataEndpoint::GetCurrentBuffer() == 0)
    {
        data = reinterpret_cast<uint8_t*>(CdcDataEndpoint::Buffer0);
    }
    else
    {
        data = reinterpret_cast<uint8_t*>(CdcDataEndpoint::Buffer1);
    }

    if(data[0] == '0')
    {
        Led::Clear();
    }
    if(data[0] == '1')
    {
        Led::Set();
    }

    CdcDataEndpoint::SwitchBuffer();
}

extern "C" void USB_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}