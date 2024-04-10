#include <clock.h>
#include <iopins.h>
#include <pinlist.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

using CdcCommEndpointBase = InEndpointBase<1, EndpointType::Interrupt, 8, 0xff>;
using CdcDataEndpointBase = BidirectionalEndpointBase<2, EndpointType::Bulk, 8, 0>;

using EpInitializer = EndpointsInitializer<DefaultEp0, CdcCommEndpointBase, CdcDataEndpointBase>;

using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using CdcCommEndpoint = EpInitializer::ExtendEndpoint<CdcCommEndpointBase>;
using CdcDataEndpoint = EpInitializer::ExtendEndpoint<CdcDataEndpointBase>;

using CdcComm = DefaultCdcCommInterface<0, Ep0, CdcCommEndpoint>;
using CdcData = CdcDataInterface<1, 0, 0, 0, Ep0, CdcDataEndpoint>;

using Config = Configuration<0, 250, false, false, CdcComm, CdcData>;

using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Comm, 0, 0, 0x0483, 0x5711, 0x0000, Ep0, Config>;

using Led = IO::Pc13Inv;

void ConfigureClock();
void ConfigureUsbPins();
void ConfigureLed();

int main()
{
    ConfigureClock();
    ConfigureUsbPins();
    ConfigureLed();

    MyDevice::Enable();

    for(;;)
    {
    }
}

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetDivider(25);
    PllClock::SetMultiplier(336);
    PllClock::SetSystemOutputDivider(PllClock::SystemOutputDivider::Div4);
    PllClock::SetUsbOutputDivider(7);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
}

void ConfigureUsbPins()
{
    Zhele::IO::Porta::Enable();

    Pa11::SetConfiguration<Pa11::Configuration::AltFunc>();
    Pa11::SetSpeed<Pa11::Speed::Fastest>();
    Pa11::AltFuncNumber<10>();

    Pa12::SetConfiguration<Pa12::Configuration::AltFunc>();
    Pa12::SetSpeed<Pa12::Speed::Fastest>();
    Pa12::AltFuncNumber<10>();

}

void ConfigureLed()
{
    Led::Port::Enable();
    Led::SetConfiguration(Pc13::Configuration::Out);
    Led::SetDriverType(Pc13::DriverType::PushPull);
    Led::Clear();
}

template<>
void CdcDataEndpoint::HandleRx()
{
    uint8_t* data = reinterpret_cast<uint8_t*>(CdcDataEndpoint::RxBuffer);
    uint8_t size = CdcDataEndpoint::BufferSize;

    if(size > 0)
    {
        if(data[0] == '0')
        {
            Led::Clear();
            CdcDataEndpoint::SendData("Led off", 7);
        }
        if(data[0] == '1')
        {
            Led::Set();
            CdcDataEndpoint::SendData("Led on", 6);
        }
    }
    CdcDataEndpoint::SetRxStatusValid();
}

extern "C" void OTG_FS_IRQHandler(void)
{   
    MyDevice::CommonHandler();
}