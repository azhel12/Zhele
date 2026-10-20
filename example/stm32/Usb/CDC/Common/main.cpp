#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/pinlist.h>
#include <zhele/usb.h>

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

#if defined(STM32F0)
using Led = Pc7;
#define USB_HANDLER USB_IRQHandler
#elif defined(STM32F1)
using Led = Pc13Inv;
#define USB_HANDLER USB_LP_IRQHandler
#elif defined(STM32F4)
using Led = Pc13Inv;
#define USB_HANDLER OTG_FS_IRQHandler
#elif defined(STM32G4)
using Led = Pc6;
#define USB_HANDLER USB_LP_IRQHandler
#elif defined(STM32H5)
using Led = Pc13;
#define USB_HANDLER USB_DRD_FS_IRQHandler
#endif

void ConfigureClock()
{
#if defined(STM32F0)
    PllClock::SelectClockSource<PllClock::ClockSource::Internal>();
    PllClock::SetMultiplier<12>();
    PllClock::SetDivider<2>();
    ApbClock::SetPrescaler<ApbClock::Div1>();
    SysClock::SelectClockSource<SysClock::Pll>();
    Hsi48Clock::Enable();
    SysCfgCompClock::Enable();
#elif defined(STM32F1)
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    PllClock::SetMultiplier<9>();
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    SysClock::SelectClockSource<SysClock::Pll>();
    MyDevice::SelectClockSource<ClockSource::PllDividedOneAndHalf>();
#elif defined(STM32F4)
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    PllClock::SetDivider<25>();
    PllClock::SetMultiplier<336>();
    PllClock::SetSystemOutputDivider<4>();
    PllClock::SetUsbOutputDivider<7>();
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    SysClock::SelectClockSource<SysClock::Pll>();
#elif defined(STM32G4) || defined(STM32H5)
    MyDevice::SelectClockSource<ClockSource::Hsi48>();
#endif
}

void ConfigureUsbPins()
{
#if defined(STM32F4) || defined(STM32H5)
    using UsbPins = PinList<Pa11, Pa12>;
    UsbPins::Enable();
    UsbPins::SetConfiguration<UsbPins::Configuration::AltFunc>();
    UsbPins::SetSpeed<UsbPins::Speed::Fastest>();
    UsbPins::AltFuncNumber<10>();
#endif
}

void ConfigureLed()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();
    Led::Clear();
}

int main()
{
    ConfigureClock();
    ConfigureLed();
    ConfigureUsbPins();

    MyDevice::Enable();

    for(;;)
    {
    }
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

extern "C" void USB_HANDLER()
{
    MyDevice::CommonHandler();
}
