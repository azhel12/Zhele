#include <clock.h>
#include <iopins.h>
#include <pinlist.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

using CdcCommEndpointBase = InEndpointBase<1, EndpointType::Interrupt, 8, 0xff>;
using CdcDataEndpointBase = BulkDoubleBufferedEndpointBase<2, EndpointDirection::Out, 64>;
using CdcDataEndpointBaseIn = BulkDoubleBufferedEndpointBase<3, EndpointDirection::In, 8>;

using EpInitializer = EndpointsInitializer<DefaultEp0, CdcCommEndpointBase, CdcDataEndpointBase, CdcDataEndpointBaseIn>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using CdcCommEndpoint = EpInitializer::ExtendEndpoint<CdcCommEndpointBase>;
using CdcDataEndpoint = EpInitializer::ExtendEndpoint<CdcDataEndpointBase>;
using CdcDataEndpointIn = EpInitializer::ExtendEndpoint<CdcDataEndpointBaseIn>;

using CdcComm = DefaultCdcCommInterface<0, Ep0, CdcCommEndpoint>;
using CdcData = CdcDataInterface<1, 0, 0, 0, Ep0, CdcDataEndpoint, CdcDataEndpointIn>;

using Config = Configuration<0, 250, false, false, CdcComm, CdcData>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Comm, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc13Inv;

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
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetMultiplier(9);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
    MyDevice::SelectClockSource(Zhele::Usb::ClockSource::PllDividedOneAndHalf);
}

void ConfigureLeds()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();
    Led::Set();
}

template<>
void CdcDataEndpoint::HandleRx(void* data, uint16_t size)
{
    uint8_t* buf = reinterpret_cast<uint8_t*>(data);

    if(size > 0)
    {

        if(buf[0] == '0')
        {
            Led::Clear();
            CdcDataEndpointIn::SendData("LED is turn off\r\n", 17);
        }
        if(buf[0] == '1')
        {
            Led::Set();
            CdcDataEndpointIn::SendData("LED is turn on\r\n", 16);
        }
    }
}

extern "C" void USB_LP_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}