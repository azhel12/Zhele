#include <zhele/adc.h>
#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/pinlist.h>
#include <zhele/timer.h>
#include <zhele/usb.h>

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
void ConfigureLeds();
void ConfigureTimer();

volatile uint16_t a = 0;

int main()

{
    ConfigureClock();
    ConfigureLeds();
    Zhele::IO::Porta::Enable();

    Adc1::Init<Adc1::AdcDivider::Div8>();

    MyDevice::Enable();

    while (!MyDevice::IsDeviceConfigured()) continue;

    ConfigureTimer();

    for(;;)
    {
    }
}

#if defined (STM32F0) // F072RB
void ConfigureClock()
{
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    PllClock::SetMultiplier<12>();
    PllClock::SetDivider<2>();
    ApbClock::SetPrescaler<ApbClock::Div1>();
    SysClock::SelectClockSource<SysClock::Pll>();

    Zhele::Clock::Hsi48Clock::Enable();
    Zhele::Clock::SysCfgCompClock::Enable();
}
#elif defined (STM32F1) // F103C8
void ConfigureClock()
{
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    PllClock::SetMultiplier<9>();
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    SysClock::SelectClockSource<SysClock::Pll>();
    MyDevice::SelectClockSource<Zhele::Usb::ClockSource::PllDividedOneAndHalf>();
}
#elif defined (STM32F4) // F401CC
void ConfigureClock()
{
    PllClock::SelectClockSource<PllClock::ClockSource::External>();
    PllClock::SetDivider<25>();
    PllClock::SetMultiplier<336>();
    PllClock::SetSystemOutputDivider<PllClock::SystemOutputDivider::Div4>();
    PllClock::SetUsbOutputDivider<7>();
    Apb1Clock::SetPrescaler<Apb1Clock::Div2>();
    SysClock::SelectClockSource<SysClock::Pll>();
}
#else
    #error "No example"
#endif

void ConfigureLeds()
{
    Led::Port::Enable();
    Led::SetConfiguration<Led::Configuration::Out>();
    Led::SetDriverType<Led::DriverType::PushPull>();
    Led::Set();
}

void ConfigureTimer()
{
    Timer3::Enable();
    Timer3::SetPrescaler(35999);
    Timer3::SetPeriod(1999);
    Timer3::EnableInterrupt();
    Timer3::Start();
}

template<>
void CdcDataEndpoint::HandleRx()
{
    uint8_t* data = reinterpret_cast<uint8_t*>(CdcDataEndpoint::RxBuffer);
    uint8_t size = CdcDataEndpoint::RxBufferCount::Get();
    CdcDataEndpoint::SetRxStatus(EndpointStatus::Valid);
}

extern "C"
{
#if defined (STM32F0)
    void USB_IRQHandler() // F072RB
#elif defined (STM32F1) // F103C8
    void USB_LP_IRQHandler()
#elif defined (STM32F4) // F401CC
    void OTG_FS_IRQHandler()
#else
    #error "No example"
#endif
    {
        // Sorry, but you must write device`s CommonHandler call by yourself.
        MyDevice::CommonHandler();
    }

    void ADC1_IRQHandler()
    {
        Adc1::IrqHandler();
    }

    void TIM3_IRQHandler()
    {
        /// Injected convert
        uint16_t value = Adc1::ReadInjected(1);

        if(Adc1::GetError() == Adc1::AdcError::NoError)        
        {
            char buffer[32];
            sprintf(buffer, "Adc: %d\r\n", value);
            CdcDataEndpoint::SendData(buffer, strlen(buffer));
        }
        if(Adc1::GetError() == Adc1::AdcError::ArgumentError)
        {
            CdcDataEndpoint::SendData("ArgumentError\r\n", 15);
        }
        if(Adc1::GetError() == Adc1::AdcError::HardwareError)
        {
            CdcDataEndpoint::SendData("HardwareError\r\n", 15);
        }
        if(Adc1::GetError() == Adc1::AdcError::NotReady)
        {
            CdcDataEndpoint::SendData("NotReady\r\n", 10);
        }
        if(Adc1::GetError() == Adc1::AdcError::Overflow)
        {
            CdcDataEndpoint::SendData("Overflow\r\n", 10);
        }
        if(Adc1::GetError() == Adc1::AdcError::RegularError)
        {
            CdcDataEndpoint::SendData("RegularError\r\n", 14);
        }
        if(Adc1::GetError() == Adc1::AdcError::TransferError)
        {
            CdcDataEndpoint::SendData("TransferError\r\n", 15);
        }

        Timer3::ClearInterruptFlag();
    }
}