// Works on Stm32f0

#include <clock.h>
#include <iopins.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

using Report = HidReport<
        0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
        0x09, 0x01,                    // USAGE (Vendor Usage 1)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x85, 0x01,                    //   REPORT_ID (1)
        0x09, 0x01,                    //   USAGE (Vendor Usage 1)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0xb1, 0x82,                    //   FEATURE (Data,Var,Abs,Vol)
        0x85, 0x01,                    //   REPORT_ID (1)
        0x09, 0x01,                    //   USAGE (Vendor Usage 1)
        0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
        0xc0                           // END_COLLECTION
    >;


using LedsControlEpBase = OutEndpointBase<1, EndpointType::Interrupt, 4, 32>;
using EpInitializer = EndpointsInitializer<DefaultEp0, LedsControlEpBase>;

using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using LedsControlEp = EpInitializer::ExtendEndpoint<LedsControlEpBase>;

using HidDesc = HidImpl<0x1001, Report>;
using Hid = HidInterface<0, 0, 0, 0, HidDesc, Ep0, LedsControlEp>;
using Config = Configuration<0, 250, false, false, Hid>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::InterfaceSpecified, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc6; // Use Pc13 for BluePill.

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
void LedsControlEp::Handler()
{
    LedsControlEp::ClearCtrRx();
    uint8_t* buffer = reinterpret_cast<uint8_t*>(LedsControlEp::Buffer);
    bool needSet = buffer[1] != 0;

    switch(buffer[0])
    {
    case 1:
        needSet ? Led::Set() : Led::Clear();
        break;
    }

    LedsControlEp::SetRxStatus(EndpointStatus::Valid);
}

extern "C" void USB_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}