/**
 * @file
 * USART methods implementation for CH32 (WCH USART peripheral).
 *
 * @author Aleksei Zhelonkin (based on the STM32 usart implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_USART_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_USART_H

namespace Zhele
{
    namespace Private
    {
        #define USART_TEMPLATE_ARGS template< \
            typename _Regs, \
            IRQn_Type _IRQNumber, \
            typename _ClockCtrl, \
            uint32_t _RemapMask, \
            uint32_t _RemapShift, \
            typename _DmaTx, \
            typename _DmaRx>
        #define USART_TEMPLATE_QUALIFIER Usart<_Regs, _IRQNumber, _ClockCtrl, _RemapMask, _RemapShift, _DmaTx, _DmaRx>

        USART_TEMPLATE_ARGS
        template<unsigned long baud>
        void USART_TEMPLATE_QUALIFIER::Init(UsartMode mode)
        {
            Init(baud, mode);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Init(unsigned baud, UsartMode mode)
        {
            _ClockCtrl::Enable();
            SetBaud(baud);
            _Regs()->STATR = 0x00;
            _Regs()->CTLR3 = mode.CTLR3;
            _Regs()->CTLR2 = mode.CTLR2;
            _Regs()->CTLR1 = mode.CTLR1 | USART_CTLR1_UE;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::SetConfig(UsartMode modeMask)
        {
            _Regs()->CTLR3 |= modeMask.CTLR3;
            _Regs()->CTLR2 |= modeMask.CTLR2;
            _Regs()->CTLR1 |= modeMask.CTLR1;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::ClearConfig(UsartMode modeMask)
        {
            _Regs()->CTLR3 &= ~modeMask.CTLR3;
            _Regs()->CTLR2 &= ~modeMask.CTLR2;
            _Regs()->CTLR1 &= ~modeMask.CTLR1;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::SetBaud(unsigned baud)
        {
            _Regs()->BRR = _ClockCtrl::ClockFreq() / baud;
        }

        USART_TEMPLATE_ARGS
        bool USART_TEMPLATE_QUALIFIER::ReadReady()
        {
            return _Regs()->STATR & RxNotEmptyInt;
        }

        USART_TEMPLATE_ARGS
        uint8_t USART_TEMPLATE_QUALIFIER::Read()
        {
            while (!ReadReady())
                ;
            return static_cast<uint8_t>(_Regs()->DATAR);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback)
        {
            _DmaRx::ClearTransferComplete();
            _Regs()->CTLR3 |= USART_CTLR3_DMAR;
            _DmaRx::SetTransferCallback(callback);
            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement, receiveBuffer, &_Regs()->DATAR, bufferSize);
        }

        USART_TEMPLATE_ARGS
        bool USART_TEMPLATE_QUALIFIER::WriteReady()
        {
            if constexpr (!std::is_same_v<_DmaTx, void>)
            {
                bool dmaActive = (_Regs()->CTLR3 & USART_CTLR3_DMAT) && _DmaTx::Enabled();
                return (!dmaActive || _DmaTx::TransferComplete()) && (_Regs()->STATR & TxEmptyInt);
            }
            else
            {
                return _Regs()->STATR & TxEmptyInt;
            }
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Write(const void* data, size_t size)
        {
            const uint8_t* ptr = static_cast<const uint8_t*>(data);
            while (size--)
                Write(*ptr++);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::WriteAsync(const void* data, size_t size, TransferCallback callback)
        {
            if (size == 0)
                return;

            while (!WriteReady())
                ;
            _DmaTx::ClearTransferComplete();
            _DmaTx::SetTransferCallback(callback);
            _Regs()->CTLR3 |= USART_CTLR3_DMAT;
            _Regs()->STATR &= ~TxCompleteInt;
            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->DATAR, size);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Write(uint8_t data)
        {
            while (!WriteReady())
                ;
            _Regs()->DATAR = data;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::EnableInterrupt(InterruptFlags interruptFlags)
        {
            uint32_t cr1Mask = 0;
            uint32_t cr2Mask = 0;
            uint32_t cr3Mask = 0;

            if (interruptFlags & ParityErrorInt)
                cr1Mask |= USART_CTLR1_PEIE;

            static_assert(
                USART_CTLR1_TXEIE  == TxEmptyInt &&
                USART_CTLR1_TCIE   == TxCompleteInt &&
                USART_CTLR1_RXNEIE == RxNotEmptyInt &&
                USART_CTLR1_IDLEIE == IdleInt);

            cr1Mask |= interruptFlags & (USART_CTLR1_TXEIE | USART_CTLR1_TCIE | USART_CTLR1_RXNEIE | USART_CTLR1_IDLEIE);

            if (interruptFlags & LineBreakInt)
                cr2Mask |= USART_CTLR2_LBDIE;

            if (interruptFlags & ErrorInt)
                cr3Mask |= USART_CTLR3_EIE;

            if (interruptFlags & CtsInt)
                cr3Mask |= USART_CTLR3_CTSIE;

            _Regs()->CTLR1 |= cr1Mask;
            _Regs()->CTLR2 |= cr2Mask;
            _Regs()->CTLR3 |= cr3Mask;

            if (interruptFlags != NoInterrupt)
                NVIC_EnableIRQ(_IRQNumber);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::DisableInterrupt(InterruptFlags interruptFlags)
        {
            uint32_t cr1Mask = 0;
            uint32_t cr2Mask = 0;
            uint32_t cr3Mask = 0;

            if (interruptFlags & ParityErrorInt)
                cr1Mask |= USART_CTLR1_PEIE;

            cr1Mask |= interruptFlags & (USART_CTLR1_TXEIE | USART_CTLR1_TCIE | USART_CTLR1_RXNEIE | USART_CTLR1_IDLEIE);

            if (interruptFlags & LineBreakInt)
                cr2Mask |= USART_CTLR2_LBDIE;

            if (interruptFlags & ErrorInt)
                cr3Mask |= USART_CTLR3_EIE;

            if (interruptFlags & CtsInt)
                cr3Mask |= USART_CTLR3_CTSIE;

            _Regs()->CTLR1 &= ~cr1Mask;
            _Regs()->CTLR2 &= ~cr2Mask;
            _Regs()->CTLR3 &= ~cr3Mask;
        }

        USART_TEMPLATE_ARGS
        typename USART_TEMPLATE_QUALIFIER::InterruptFlags USART_TEMPLATE_QUALIFIER::InterruptSource()
        {
            return static_cast<InterruptFlags>(_Regs()->STATR & InterruptMask);
        }

        USART_TEMPLATE_ARGS
        typename USART_TEMPLATE_QUALIFIER::Error USART_TEMPLATE_QUALIFIER::GetError()
        {
            return static_cast<Error>(_Regs()->STATR & ErrorMask);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::ClearInterruptFlag(InterruptFlags interruptFlags)
        {
            _Regs()->STATR &= ~interruptFlags;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::ClearAllInterruptFlags()
        {
            _Regs()->STATR = 0x00000000;
        }

        USART_TEMPLATE_ARGS
        template<typename TxPin, typename RxPin, uint8_t Remap>
        void USART_TEMPLATE_QUALIFIER::SelectTxRxPins()
        {
            TxPin::Port::Enable();
            TxPin::template SetConfiguration<TxPin::Port::Configuration::AltFunc>();
            TxPin::template SetDriverType<TxPin::Port::DriverType::PushPull>();
            TxPin::template SetSpeed<TxPin::Port::Speed::Fast>();

            if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>)
                RxPin::Port::Enable();
            RxPin::template SetConfiguration<RxPin::Port::Configuration::In>();
            RxPin::template SetPullMode<RxPin::Port::PullMode::PullUp>();

            if constexpr (_RemapMask != 0)
            {
                Clock::Apb2PeriphClockEnable::Or(RCC_AFIOEN);
                AFIO->PCFR1 = (AFIO->PCFR1 & ~_RemapMask) | ((static_cast<uint32_t>(Remap) << _RemapShift) & _RemapMask);
            }
        }

        #undef USART_TEMPLATE_ARGS
        #undef USART_TEMPLATE_QUALIFIER
    }
}
#endif // ZHELE_PLATFORM_CH32_COMMON_IMPL_USART_H
