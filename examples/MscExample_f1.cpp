#include <clock.h>
#include <iopins.h>
#include <pinlist.h>
#include <usb.h>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;
using namespace Zhele::Usb;

constexpr Zhele::TemplateUtils::fixed_string_16 Manufacturer(u"ZheleProduction");
constexpr Zhele::TemplateUtils::fixed_string_16 Product(u"MSDExample");
constexpr Zhele::TemplateUtils::fixed_string_16 Serial(u"88005553535");

using MscOutEpBase = BulkDoubleBufferedEndpointBase<1, EndpointDirection::Out, 64>;
using MscInEpBase = InBulkDoubleBufferedWithoutZlpEndpointBase<2, 64>;

using EpInitializer = EndpointsInitializer<DefaultEp0, MscOutEpBase, MscInEpBase>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using MscOutEp = EpInitializer::ExtendEndpoint<MscOutEpBase>;
using MscInEp = EpInitializer::ExtendEndpoint<MscInEpBase>;

using Lun0 = DefaultScsiLun<512, 36>;

using Scsi = ScsiBulkInterface<0, 0, Ep0, MscOutEp, MscInEp, Lun0>;

using Config = Configuration<0, 250, false, false, Scsi>;
using MyDevice = DeviceWithStrings<0x0200, DeviceAndInterfaceClass::Storage, 0, 0, 0x0483, 0x5711, 0, Manufacturer, Product, Serial, Ep0, Config>;

void ConfigureClock();
void ConfigureI2c();

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
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetMultiplier(9);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
    MyDevice::SelectClockSource(Zhele::Usb::ClockSource::PllDividedOneAndHalf);
}

template<>
void MscOutEp::HandleRx(void* data, uint16_t size)
{
    Scsi::HandleRx(data, size);
}

extern "C" void USB_LP_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}