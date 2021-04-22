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

#include <clock.h>

using namespace Zhele;

int main(){}

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

    Clock::AdcClockSource::SelectClockSource();
    Clock::AdcClockSource::SetPrescaler(Clock::AdcClockSource::Prescaler());
    Clock::AdcClockSource::SrcClockFreq();
    Clock::AdcClockSource::ClockFreq();

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
    using DmaCh = Dma1Channel1;

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
    DmaCh::Interrupt();
    DmaCh::ClearFlags();
    DmaCh::ClearTransferError();
    DmaCh::ClearHalfTransfer();
    DmaCh::ClearTransferComplete();
    DmaCh::ClearInterrupt();
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

