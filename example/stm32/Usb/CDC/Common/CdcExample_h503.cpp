// USB CDC example for STM32H503 (USB_DRD_FS controller).
//
// The board is expected to expose the USB connector on PA11 (DM) / PA12 (DP)
// and a LED on PC13. The device echoes a message when '0' or '1' is received.

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

using Led = IO::Pc13;

void ConfigureLed();
void ConfigureUsbPins();

int main()
{
    ConfigureLed();
    ConfigureUsbPins();

    MyDevice::SelectClockSource<ClockSource::Hsi48>();
    MyDevice::Enable();

    for(;;)
    {
    }
}

void ConfigureLed()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();
    Led::Clear();
}

void ConfigureUsbPins()
{
    // Unlike F0/F1, on H5 the transceiver is reached through the alternate function.
    using UsbPins = PinList<Pa11, Pa12>;
    UsbPins::Enable();
    UsbPins::SetConfiguration<UsbPins::Configuration::AltFunc>();
    UsbPins::AltFuncNumber<10>();
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

extern "C" void USB_DRD_FS_IRQHandler()
{
    // Sorry, but you must write device`s CommonHandler call by yourself.
    MyDevice::CommonHandler();
}
