#include <clock.h>
#include <exti.h>
#include <iopins.h>
#include <pinlist.h>
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
        0xc0                            // END_COLLECTION
    >;

using HidDesc = HidDescriptor<0x0200, Report>;

using LedsControlEpBase = OutEndpointBase<1, EndpointType::Interrupt, 64, 32>;
using EpInitializer = EndpointsInitializer<DefaultEp0, LedsControlEpBase>;

using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using LedsControlEp = EpInitializer::ExtendEndpoint<LedsControlEpBase>;

using Hid = HidInterface<0, 0, 0, 0, HidDesc, LedsControlEp>;
using Config = HidConfiguration<0, 250, false, false, Report, Hid>;
using MyDevice = Device<0x0200, DeviceClass::InterfaceSpecified, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

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
    Portc::Enable();
    using Leds = PinList<Pc6, Pc7, Pc8, Pc9>;
    Leds::Enable();
    Leds::SetConfiguration<Leds::Configuration::Out>();
    Leds::SetDriverType<Leds::DriverType::PushPull>();
    Leds::Set(0xff);
}

void ConfigureExti()
{
    Exti0::Init<Exti0::Trigger::Rising, Porta>();
    Exti0::InitPin<Pa0>(Pa0::PullMode::PullDown);
    Exti0::EnableInterrupt();
}

template<>
void LedsControlEp::Handler()
{
    //Pc7::Toggle();
    LedsControlEp::ClearCtrRx();
    uint8_t* buffer = reinterpret_cast<uint8_t*>(LedsControlEp::Buffer);
    bool needSet = buffer[1] != 0;

    switch(buffer[0])
    {
    case 1:
        needSet ? IO::Pc6::Set() : IO::Pc6::Clear();
        break;
    case 2:
        needSet ? IO::Pc7::Set() : IO::Pc7::Clear();
        break;
    case 3:
        needSet ? IO::Pc8::Set() : IO::Pc8::Clear();
        break;
    case 4:
        needSet ? IO::Pc9::Set() : IO::Pc9::Clear();
        break;
    }

    LedsControlEp::SetRxStatus(EndpointStatus::Valid);
}

extern "C" void USB_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}

uint16_t buffer[256];

extern "C" void EXTI0_1_IRQHandler()
{
    /*
    uint16_t* src = reinterpret_cast<uint16_t*>(USB_PMAADDR);
    for(int i = 0; i < 256; ++i)
    {
        buffer[i] = src[i];
    }

    volatile uint32_t bufferAddres = reinterpret_cast<uint32_t>(buffer);
    volatile int a = 42;

    Exti0::ClearInterruptFlag();
    */
}