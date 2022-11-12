#include <clock.h>
#include <iopins.h>
#include <pinlist.h>
#include <usb.h>

#include <usart.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

constexpr Zhele::TemplateUtils::fixed_string_16 Manufacturer(u"ZheleProduction");
constexpr Zhele::TemplateUtils::fixed_string_16 Product(u"MSDExample");
constexpr Zhele::TemplateUtils::fixed_string_16 Serial(u"88005553535");

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

// Конечные точки
using MscOutEpBase = OutEndpointBase<1, EndpointType::Bulk, 64, 0>;
using MscInEpBase = InEndpointWithoutZlpBase<2, EndpointType::Bulk, 64, 0>;
using LedsControlEpBase = OutEndpointBase<3, EndpointType::Interrupt, 4, 255>;

// Расширение конечных точек
using EpInitializer = EndpointsInitializer<DefaultEp0, MscOutEpBase, MscInEpBase, LedsControlEpBase>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;
using MscOutEp = EpInitializer::ExtendEndpoint<MscOutEpBase>;
using MscInEp = EpInitializer::ExtendEndpoint<MscInEpBase>;
using LedsControlEp = EpInitializer::ExtendEndpoint<LedsControlEpBase>;

// Интерфейс MSC
using Lun0 = DefaultScsiLun<512, 120>;
using Scsi = ScsiBulkInterface<0, 0, Ep0, MscOutEp, MscInEp, Lun0>;
// Интерфейс HID
using Hid = HidInterface<1, 0, 0, 0, HidDesc, Ep0, LedsControlEp>;

using Config = Configuration<0, 250, false, false, Scsi, Hid>; // Просто вписываем два интерфейса
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::InterfaceSpecified, 0x02, 0, 0x0483, 0x5711, 0, Ep0, Config>;

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
    Pc13Inv::Port::Enable();
    Pc13Inv::SetConfiguration(Pc13Inv::Configuration::Out);
    Pc13Inv::SetDriverType(Pc13Inv::DriverType::PushPull);
    Pc13Inv::Clear();
}

template<>
void MscOutEp::HandleRx()
{
    uint8_t* data = reinterpret_cast<uint8_t*>(MscOutEp::Buffer);
    uint8_t size = MscOutEp::BufferSize;

    Scsi::HandleRx(data, size);

    MscOutEp::SetRxStatusValid();
}

template<>
void LedsControlEp::Handler()
{
    uint8_t* buffer = reinterpret_cast<uint8_t*>(LedsControlEp::Buffer);
    bool needSet = buffer[1] != 0;

    switch(buffer[0])
    {
    case 1:
        needSet ? Pc13Inv::Set() : Pc13Inv::Clear();
        break;
    }

    LedsControlEp::SetRxStatus(EndpointStatus::Valid);
}

extern "C" void OTG_FS_IRQHandler(void)
{   
    MyDevice::CommonHandler();
}