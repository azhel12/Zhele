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

using MscOutEpBase = OutEndpointBase<1, EndpointType::Bulk, 64, 0>;
using MscInEpBase = InEndpointWithoutZlpBase<2, EndpointType::Bulk, 64, 0>;

using EpInitializer = EndpointsInitializer<DefaultEp0, MscOutEpBase, MscInEpBase>;
using Ep0 = EpInitializer::ExtendEndpoint<DefaultEp0>;

using MscOutEp = EpInitializer::ExtendEndpoint<MscOutEpBase>;
using MscInEp = EpInitializer::ExtendEndpoint<MscInEpBase>;

using Lun0 = ScsiLunWithConstSize<512, 131072>;

using Scsi = ScsiBulkInterface<0, 0, Ep0, MscOutEp, MscInEp, Lun0>;

using Config = Configuration<0, 250, false, false, Scsi>;
using MyDevice = Device<0x0200, DeviceAndInterfaceClass::Storage, 0, 0, 0x0483, 0x5711, 0, Ep0, Config>;

void ConfigureClock();
void ConfigureUsbPins();

int main()
{
    ConfigureClock();
    ConfigureUsbPins();

    MyDevice::Enable();

    for(;;)
    {
    }
}

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetDivider(HSE_VALUE / 1000000);
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

template<>
void MscOutEp::HandleRx()
{
    uint8_t* data = reinterpret_cast<uint8_t*>(MscOutEp::Buffer);
    uint8_t size = MscOutEp::BufferSize;

    Scsi::HandleRx(data, size);

    MscOutEp::SetRxStatusValid();
}

extern "C" void OTG_FS_IRQHandler(void)
{   
    MyDevice::CommonHandler();
}

const int RealLbaCount = 48;
uint8_t LunBuffer[512 * (RealLbaCount + 1)];
uint32_t RxAddress;
uint32_t BytesRemain;

template<>
bool Lun0::Write10Handler(uint32_t startLba, uint32_t lbaCount)
{
    RxAddress = startLba * 512;
    BytesRemain = lbaCount * 512;
    
    return lbaCount > 0;
}
template<>
bool Lun0::RxHandler(void* data, uint16_t size)
{
    if(RxAddress < 512 * RealLbaCount) {
        CopyFromUsbPma(&LunBuffer[RxAddress], data, size);
    } else {
        CopyFromUsbPma(&LunBuffer[512 * RealLbaCount], data, size);
    }

    RxAddress += size;
    BytesRemain -= size;

    return BytesRemain > 0;
}

uint32_t lba;
int32_t count = 0;
InTransferCallback cbk;

template<typename _InEp>
void SendNext()
{
    if(count > 1) {
        --count;
        _InEp::SendData(&LunBuffer[512 * RealLbaCount], 512, SendNext<_InEp>);
    } else {
        _InEp::SendData(&LunBuffer[512 * RealLbaCount], 512, cbk);
    }
}

template<>
template<typename _InEp>
void Lun0::Read10Handler(uint32_t startLba, uint32_t lbaCount, InTransferCallback callback)
{
    cbk = callback;
    lba = startLba;
    count = lbaCount;
    
    if(lba > RealLbaCount) {
        SendNext<_InEp>();
    } else {
        if(RealLbaCount > (lba + count)) {
            _InEp::SendData(&LunBuffer[512 * lba], count * 512, cbk);
        } else {
            count -= (RealLbaCount - lba);
            _InEp::SendData(&LunBuffer[512 * lba], (RealLbaCount - lba) * 512, SendNext<_InEp>);
        }
    }
}