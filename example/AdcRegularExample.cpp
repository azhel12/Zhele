#define F_CPU 72000000

#include <adc.h>
#include <clock.h>
#include <iopins.h>
#include <pinlist.h>
#include <timer.h>
#include <usb.h>

#include <cstdio> // For sprintf

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Timers;
using namespace Zhele::Usb;

using CdcCommEndpointBase = InEndpointBase<1, EndpointType::Interrupt, 8, 0xff>;
using CdcDataEndpointBase = BidirectionalEndpointBase<2, EndpointType::Bulk, 32, 0>;

using EpInitializer = EndpointsInitializer<DefaultEp0, CdcCommEndpointBase, CdcDataEndpointBase>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using CdcCommEndpoint = EpInitializer::ExtendEndpoint<CdcCommEndpointBase>;
using CdcDataEndpoint = EpInitializer::ExtendEndpoint<CdcDataEndpointBase>;

using CdcComm = DefaultCdcCommInterface<0, Ep0, CdcCommEndpoint>;
using CdcData = CdcDataInterface<1, 0, 0, 0, Ep0, CdcDataEndpoint>;

using Config = Configuration<0, 250, false, false, CdcComm, CdcData>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Comm, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc13Inv;

void ConfigureClock();
void ConfigureTimer();

void AdcRegularCallback(uint16_t *data, uint32_t count);

uint16_t Buffer[3];

int main()
{
    ConfigureClock();

    Zhele::IO::Porta::Enable();

    Adc1::Init(Adc1::AdcDivider::Div8);

    MyDevice::Enable();

    while (!MyDevice::IsDeviceConfigured())
    {
    }

    Adc1::SetRegularCallback(AdcRegularCallback);

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

template<>
void CdcDataEndpoint::HandleRx()
{
    Adc1::StartRegular({0, 1, 2}, Buffer, 1);
    CdcDataEndpoint::SetRxStatus(EndpointStatus::Valid);
}

void AdcRegularCallback(uint16_t* data, uint32_t count)
{
    char buffer[64];
    sprintf(buffer, "Ch0:%d\r\nCh1:%d\r\nCh2:%d\r\n", Buffer[0], Buffer[1], Buffer[2]);
    CdcDataEndpoint::SendData(buffer, strlen(buffer));
}

extern "C"
{
    void USB_LP_IRQHandler()
    {
        // Sorry, but you must write device`s CommonHandler call by yourself.
        MyDevice::CommonHandler();
    }

    void ADC1_IRQHandler()
    {
        Adc1::IrqHandler();
    }
}