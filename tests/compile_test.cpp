/**
 * @file
 * Implements compile tests.
 * Templates (functions and classes) are compiled only if they are instanced.
 * So, for check that all code is correct for compiler, I should call all template methods.
 * 
 * @author Aleksei Zhelonkin.
 * @date 2021
 * @license FreeBSD
 */


int main(){}

#include <clock.h>
using namespace Zhele;

void ClockCompileTest()
{
    Clock::HseClock::SrcClockFreq();
    Clock::HseClock::GetDivider();
    Clock::HseClock::GetMultipler();
    Clock::HseClock::ClockFreq();
    Clock::HseClock::Enable();   
    Clock::HseClock::Disable();

    Clock::HsiClock::SrcClockFreq();
    Clock::HsiClock::GetDivider();
    Clock::HsiClock::GetMultipler();
    Clock::HsiClock::ClockFreq();
    Clock::HsiClock::Enable();   
    Clock::HsiClock::Disable();

    Clock::PllClock::SetClockFreq(ClockFrequenceT());
    Clock::PllClock::SrcClockFreq();
    Clock::PllClock::GetDivider();
    Clock::PllClock::SetDivider(ClockFrequenceT());
    Clock::PllClock::GetMultipler();   
    Clock::PllClock::SetMultiplier(ClockFrequenceT());
#if defined(RCC_PLLCFGR_PLLP)
    Clock::PllClock::GetSystemOutputDivider();
    Clock::PllClock::SetSystemOutputDivider(ClockFrequenceT());
    Clock::PllClock::GetUsbOutputDivider();
    Clock::PllClock::SetUsbOutputDivider(ClockFrequenceT());
#endif
    Clock::PllClock::SelectClockSource(Clock::PllClock::ClockSource());
    Clock::PllClock::GetClockSource();
    Clock::PllClock::ClockFreq();
    Clock::PllClock::Enable();
    Clock::PllClock::Disable();

#if defined (RCC_CSR_LSION)
    Clock::LsiClock::SrcClockFreq();
    Clock::LsiClock::GetDivider();
    Clock::LsiClock::GetMultipler();
    Clock::LsiClock::ClockFreq();
    Clock::LsiClock::Enable();
    Clock::LsiClock::Disable();
#endif

    Clock::SysClock::MaxFreq();
    Clock::SysClock::SelectClockSource(Clock::SysClock::ClockSource());
    Clock::SysClock::ClockFreq();
    Clock::SysClock::SrcClockFreq();
    Clock::SysClock::SetClockFreq(ClockFrequenceT());

    Clock::AhbClock::ClockFreq();
    Clock::AhbClock::SetPrescaler(Clock::AhbClock::Prescaler());

    Clock::Apb1Clock::ClockFreq();
    Clock::Apb1Clock::SetPrescaler(Clock::Apb1Clock::Prescaler());

    Clock::Apb2Clock::ClockFreq();
    Clock::Apb2Clock::SetPrescaler(Clock::Apb2Clock::Prescaler());
#if defined (RCC_CFGR_ADCPRE)
    Clock::AdcClockSource::SelectClockSource();
    Clock::AdcClockSource::SetPrescaler(Clock::AdcClockSource::Prescaler());
    Clock::AdcClockSource::SrcClockFreq();
    Clock::AdcClockSource::ClockFreq();
#endif

#if defined(RCC_HSI48_SUPPORT)
    Clock::Hsi48Clock::SrcClockFreq();
    Clock::Hsi48Clock::GetDivider();
    Clock::Hsi48Clock::GetMultipler();
    Clock::Hsi48Clock::ClockFreq();
    Clock::Hsi48Clock::Enable();
    Clock::Hsi48Clock::Disable();
#endif
}

#include <dma.h>

void DmaCompileTest()
{
#if defined (DMA1_Stream0)
    using DmaCh = Dma1Stream0;
#else
    using DmaCh = Dma1Channel1;
#endif
    DmaCh::Transfer(DmaCh::Mode(), nullptr, nullptr, 0);
    DmaCh::SetTransferCallback(nullptr);
    DmaCh::Ready();
    DmaCh::Enabled();
    DmaCh::Enable();
    DmaCh::Disable();
    DmaCh::RemainingTransfers();
    DmaCh::PeriphAddress();
    DmaCh::MemAddress();
    DmaCh::TransferError();
    DmaCh::HalfTransfer();
    DmaCh::TransferComplete();
#if defined(DMA_CCR_EN)
    DmaCh::Interrupt();
#endif
    DmaCh::ClearFlags();
    DmaCh::ClearTransferError();
    DmaCh::ClearHalfTransfer();
    DmaCh::ClearTransferComplete();
#if defined(DMA_CCR_EN)
    DmaCh::ClearInterrupt();
#endif
    DmaCh::IrqHandler();

    using DmaMod = Dma1;

    DmaMod::TransferError<0>();
    DmaMod::HalfTransfer<0>();
    DmaMod::TransferComplete<0>();
#if defined(DMA_SxCR_EN)
    DmaMod::FifoError<0>();
    DmaMod::DirectError<0>();
    DmaMod::ClearFifoError<0>();
    DmaMod::ClearDirectError<0>();
#endif
#if defined(DMA_CCR_EN)
    DmaMod::Interrupt<0>();
    DmaMod::ClearInterrupt<0>();
#endif
    DmaMod::ClearChannelFlags<0>();
    DmaMod::ClearTransferError<0>();
    DmaMod::ClearHalfTransfer<0>();
    DmaMod::ClearTransferComplete<0>();
    DmaMod::Enable();
    DmaMod::Disable();
}

#include <i2c.h>
void I2cCompileTest()
{
    using I2c = I2c1;

    I2c::Init();
    I2c::WriteU8(0, 0, 0);
    I2c::Write(0, 0, nullptr, 0);
    I2c::WriteAsync(0, 0, nullptr, 0);
    I2c::ReadU8(0, 0);
    I2c::Read(0, 0, nullptr, 0);
    I2c::EnableAsyncRead(0, 0, nullptr, 0);
    I2c::WriteRegAddr(0, I2cOpts());
    I2c::WaitEvent(0);
    I2c::Busy();
    I2c::WaitWhileBusy();
    I2c::EventIrqHandler();
    I2c::ErrorIrqHandler();
    I2c::GetErorFromEvent(0);
    I2c::SelectPins<0, 0>();
    I2c::SelectPins(0, 0);
    I2c::SelectPins<IO::Pb6, IO::Pb7>();
}

#include <ioports.h>
void IoPortsCompileTest()
{
    using Port = IO::Porta;

    Port::Read();
    Port::Write(0);
    Port::ClearAndSet(0, 0);
    Port::Set(0);
    Port::Clear(0);
    Port::Toggle(0);
    Port::PinRead();
    Port::ClearAndSet<0, 0>();
    Port::Toggle<0>();
    Port::Set<0>();
    Port::Clear<0>();
    Port::SetConfiguration(0, Port::Configuration::Analog);
    Port::SetConfiguration<0, Port::Configuration::Analog>();
    Port::SetSpeed(0, Port::Speed::Slow);
    Port::SetSpeed<0, Port::Speed::Slow>();
    Port::SetPullMode(0, Port::PullMode::NoPull);
    Port::SetPullMode<0, Port::PullMode::NoPull>();
    Port::SetDriverType(0, Port::DriverType::PushPull);
    Port::SetDriverType<0, Port::DriverType::PushPull>();
    Port::AltFuncNumber(0, 0);
    Port::AltFuncNumber<0, 0>();
    Port::Enable();
    Port::Disable();
}

#include <iopins.h>

#include <pinlist.h>
void PinListCompileTest()
{
    using Pins = IO::PinList<IO::Pa0, IO::Pb0>;    

    Pins::Enable();
    Pins::Write(0);
    Pins::Write<0>();
    Pins::Read();
    Pins::Set(0);
    Pins::Clear(0);
    Pins::SetConfiguration(0, Pins::Configuration::Analog);
    Pins::SetConfiguration<0, Pins::Configuration::Analog>();
    Pins::SetConfiguration<Pins::Configuration::Analog>();
    Pins::SetSpeed(0, Pins::Speed::Slow);
    Pins::SetSpeed<0, Pins::Speed::Slow>();
    Pins::SetSpeed<Pins::Speed::Slow>();
    Pins::SetPullMode(0, Pins::PullMode::NoPull);
    Pins::SetPullMode<0, Pins::PullMode::NoPull>();
    Pins::SetPullMode<Pins::PullMode::NoPull>();
    Pins::SetDriverType(0, Pins::DriverType::PushPull);
    Pins::SetDriverType<0, Pins::DriverType::PushPull>();
    Pins::SetDriverType<Pins::DriverType::PushPull>();
    Pins::AltFuncNumber(0, 0);
    Pins::AltFuncNumber<0, 0>();
    Pins::AltFuncNumber<0>();
    Pins::IndexOf<IO::Pa0>;
    using pin = Pins::Pin<0>;
}

#include <spi.h>
void SpiCompileTest()
{
    using SpiBus = Spi1;

    SpiBus::Enable();
    SpiBus::Disable();
    SpiBus::Init();
    SpiBus::SetDivider(SpiBus::ClockDivider::Slow);
    SpiBus::SetClockPolarity(SpiBus::ClockPolarity::ClockPolarityLow);
    SpiBus::SetClockPhase(SpiBus::ClockPhase::ClockPhaseLeadingEdge);
    SpiBus::SetBitOrder(SpiBus::BitOrder::MsbFirst);
    SpiBus::SetDataSize(SpiBus::DataSize::DataSize8);
    SpiBus::SetSlaveControl(SpiBus::SlaveControl::SoftSlaveControl);
    SpiBus::SetSS();
    SpiBus::Send(0);
    SpiBus::SendAsync(nullptr, nullptr, 0);
    SpiBus::Write(0);
    SpiBus::WriteAsync(nullptr, 0);
    SpiBus::Read();
    SpiBus::ReadAsync(nullptr, 0);
    SpiBus::SelectPins(0, 0, 0, 0);
    SpiBus::SelectPins<0, 0, 0, 0>();
}

#include <timer.h>
void TimerCompileTest()
{
    using Tim = Timers::Timer3;
    Tim::GetClockFreq();
    Tim::Enable();
    Tim::Disable();
    Tim::SetCounterValue(0);
    Tim::GetCounterValue();
    Tim::ResetCounterValue();
    Tim::SetPrescaler(0);
    Tim::GetPrescaler();
    Tim::SetPeriod(0);
    Tim::SetPeriodAndUpdate(0);
    Tim::GetPeriod();
    Tim::Stop();
    Tim::Start();
    Tim::EnableInterrupt();
    Tim::DisableInterrupt();
    Tim::IsInterrupt();
    Tim::ClearInterruptFlag();
    Tim::DmaRequestEnable();
    Tim::DmaRequestDisable();

    using TimOC = Tim::OutputCompare<0>;
    TimOC::SetPulse(0);
    TimOC::GetPulse();
    TimOC::EnableInterrupt();
    TimOC::DisableInterrupt();
    TimOC::IsInterrupt();
    TimOC::ClearInterruptFlag();
    TimOC::Enable();
    TimOC::Disable();
    TimOC::SetOutputPolarity(TimOC::OutputPolarity::ActiveHigh);
    TimOC::SetOutputMode(TimOC::OutputMode::PWM1);
    TimOC::SelectPins(0);
    TimOC::SelectPins<0>();

    using TimPWM = Tim::PWMGeneration<0>;
    TimPWM::SetOutputFastMode(TimPWM::FastMode::Disable);
    TimPWM::SelectPins(0);
    TimPWM::SelectPins<0>();
}

#include <usart.h>
void UsartCompileTest()
{
    using UsartBus = Usart1;

    UsartBus::Init<9600>();
    UsartBus::Init(9600);
    UsartBus::SetConfig(UsartBus::UsartMode::DataBits8 | UsartBus::UsartMode::FullDuplex);
    UsartBus::ClearConfig(UsartBus::UsartMode::DataBits8 | UsartBus::UsartMode::FullDuplex);
    UsartBus::SetBaud(9600);
    UsartBus::ReadReady();
    UsartBus::Read();
    UsartBus::EnableAsyncRead(nullptr, 0);
    UsartBus::WriteReady();
    UsartBus::Write(nullptr, 0);
    UsartBus::Write(0);
    UsartBus::EnableInterrupt(UsartBus::InterruptFlags::AllInterrupts);
    UsartBus::DisableInterrupt(UsartBus::InterruptFlags::AllInterrupts);
    UsartBus::InterruptSource();
    UsartBus::GetError();
    UsartBus::ClearInterruptFlag(UsartBus::InterruptFlags::AllInterrupts);
    UsartBus::SelectTxRxPins(0, 0);
    UsartBus::SelectTxRxPins<0, 0>();
}

#include <one_wire.h>
void OneWireCompileTest()
{
    using OneWireLine = OneWire<Usart1, IO::Pa9>;

    OneWireLine::Init();
    OneWireLine::Reset();
    OneWireLine::WriteByte(0);
    OneWireLine::ReadByte();
    OneWireLine::ReadBytes(nullptr, 0);
    OneWireLine::MatchRom(nullptr);
    OneWireLine::SkipRom();
    OneWireLine::ReadRom(nullptr);
    OneWireLine::SearchFirst(nullptr);
}

