#define F_CPU 84000000

#include <clock.h>
#include <exti.h>
#include <iopins.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

void ConfigureClock();
void ConfigureUsbPins();
void ConfigureLed();

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

using HidDesc = HidImpl<0x1001, Report>;

using LedsControlEpBase = OutEndpointBase<1, EndpointType::Interrupt, 4, 255>;
using EpInitializer = EndpointsInitializer<DefaultEp0, LedsControlEpBase>;

using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using LedsControlEp = EpInitializer::ExtendEndpoint<LedsControlEpBase>;

using Hid = HidInterface<0, 0, 0, 0, HidDesc, Ep0, LedsControlEp>;
using Config = Configuration<0, 250, false, false, Hid>;
using MyDevice = Device<0x0110, DeviceAndInterfaceClass::InterfaceSpecified, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

using Led = IO::Pc13;

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
    Pc13::Port::Enable();
	Pc13::SetConfiguration(Pc13::Configuration::Out);
	Pc13::SetDriverType(Pc13::DriverType::PushPull);
	Pc13::Set();
}


template<>
void LedsControlEp::Handler()
{
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

extern "C" void OTG_FS_IRQHandler(void)
{   
    MyDevice::CommonHandler();
}