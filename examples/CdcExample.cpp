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
using CdcDataEndpointBase = BidirectionalEndpointBase<2, EndpointType::Bulk, 32, 0>;

using EpInitializer = EndpointsInitializer<DefaultEp0, CdcCommEndpointBase, CdcDataEndpointBase>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using CdcCommEndpoint = EpInitializer::ExtendEndpoint<CdcCommEndpointBase>;
using CdcDataEndpoint = EpInitializer::ExtendEndpoint<CdcDataEndpointBase>;

using CdcComm = DefaultCdcCommInterface<0, Ep0, CdcCommEndpoint>;
using CdcData = CdcDataInterface<1, 0, 0, 0, Ep0, CdcDataEndpoint>;

using Config = Configuration<0, 250, false, false, CdcComm, CdcData>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Comm, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc7;

void ConfigureClock();
void ConfigureLeds();
void ConfigureExti();

int main()

{
    ConfigureClock();
    ConfigureLeds();
    ConfigureExti();
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

void ConfigureExti()
{
    Exti0::Init();
    Exti0::InitPin<IO::Pa0>(Pa0::PullMode::PullDown, Pa0::DriverType::PushPull, Pa0::Speed::Slow);
    Exti0::EnableInterrupt();
}

template<>
void CdcDataEndpoint::HandleRx()
{
    uint8_t* data = reinterpret_cast<uint8_t*>(CdcDataEndpoint::RxBuffer);
    uint8_t size = CdcDataEndpoint::RxBufferCount::Get();

    if(size > 0)
    {
        if(data[0] == '0')
        {
            Led::Clear();
            CdcDataEndpoint::SendData("LED is turn off\r\n", 17);
        }
        if(data[0] == '1')
        {
            Led::Set();
            CdcDataEndpoint::SendData("LED is turn on\r\n", 16);
        }
    }
    CdcDataEndpoint::SetRxStatus(EndpointStatus::Valid);
}

extern "C" void USB_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}

extern "C" void EXTI0_1_IRQHandler()
{
    CdcDataEndpoint::SendData("Button was pressed\r\n", 20);
    Exti0::ClearInterruptFlag();
}
