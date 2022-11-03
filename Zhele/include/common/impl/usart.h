/**
 * @file
 * UART methods implementation.
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_UART_IMPL_COMMON_H
#define ZHELE_UART_IMPL_COMMON_H

namespace Zhele
{
    UsartBase::UsartMode::_CR1 operator | (UsartBase::UsartMode::_CR1 first, UsartBase::UsartMode::_CR1 second)
    {
        return static_cast<UsartBase::UsartMode::_CR1>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
    }

    UsartBase::UsartMode::_CR2 operator | (UsartBase::UsartMode::_CR2 first, UsartBase::UsartMode::_CR2 second)
    {
        return static_cast<UsartBase::UsartMode::_CR2>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
    }

    UsartBase::UsartMode::_CR3 operator | (UsartBase::UsartMode::_CR3 first, UsartBase::UsartMode::_CR3 second)
    {
        return static_cast<UsartBase::UsartMode::_CR3>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
    }

    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR1 cr1, UsartBase::UsartMode::_CR2 cr2)
    {
        return UsartBase::UsartMode{cr1, cr2, UsartBase::UsartMode::_CR3()};
    }

    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR2 cr2, UsartBase::UsartMode::_CR1 cr1)
    {
        return cr1 | cr2;
    }

    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR2 cr2, UsartBase::UsartMode::_CR3 cr3)
    {
        return UsartBase::UsartMode{UsartBase::UsartMode::_CR1(), cr2, cr3};
    }
    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR3 cr3, UsartBase::UsartMode::_CR2 cr2)
    {
        return cr2 | cr3;
    }

    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR1 cr1, UsartBase::UsartMode::_CR3 cr3)
    {
        return UsartBase::UsartMode{cr1, UsartBase::UsartMode::_CR2(), cr3};
    }
    UsartBase::UsartMode operator | (UsartBase::UsartMode::_CR3 cr3, UsartBase::UsartMode::_CR1 cr1)
    {
        return cr1 | cr3;
    }

    namespace Private
    {
        #define USART_TEMPLATE_ARGS template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        #define USART_TEMPLATE_QUALIFIER Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>

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
            _Regs()->STATUS_REG = 0x00;
            _Regs()->CR3 = mode.CR3;
            _Regs()->CR2 = mode.CR2;
            _Regs()->CR1 = mode.CR1 | USART_CR1_UE;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::SetConfig(UsartMode modeMask)
        {
            _Regs()->CR3 |= modeMask.CR3;
            _Regs()->CR2 |= modeMask.CR2;
            _Regs()->CR1 |= modeMask.CR1;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::ClearConfig(UsartMode modeMask)
        {
            _Regs()->CR3 &= ~modeMask.CR3;
            _Regs()->CR2 &= ~modeMask.CR2;
            _Regs()->CR1 &= ~modeMask.CR1;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::SetBaud(unsigned baud)
        {
            _Regs()->BRR = _ClockCtrl::ClockFreq() / baud;
        }

        USART_TEMPLATE_ARGS
        bool USART_TEMPLATE_QUALIFIER::ReadReady()
        {
            return _Regs()->STATUS_REG & RxNotEmptyInt;
        }

        USART_TEMPLATE_ARGS
        uint8_t USART_TEMPLATE_QUALIFIER::Read()
        {
            while(!ReadReady())
                ;
            return _Regs()->RECEIVE_DATA_REG;
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback)
        {
            _DmaRx::ClearTransferComplete();
            _Regs()->CR3 |= USART_CR3_DMAR;
            _DmaRx::SetTransferCallback(callback);
            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, receiveBuffer, &_Regs()->RECEIVE_DATA_REG, bufferSize);
        }

        USART_TEMPLATE_ARGS
        bool USART_TEMPLATE_QUALIFIER::WriteReady()
        {
            bool dmaActive = (_Regs()->CR3 & USART_CR3_DMAT) && _DmaTx::Enabled();
            return (!dmaActive || _DmaTx::TransferComplete()) && (_Regs()->STATUS_REG & TxEmptyInt);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Write(const void* data, size_t size, bool async)
        {
            if (async && size > 1) {
                WriteAsync(data, size);
            }  else {
                Write(data, size);
            }
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Write(const void* data, size_t size)
        {
            const uint8_t *ptr = static_cast<const uint8_t*>(data);
            while (size--) {
                Write(*ptr++);
            }
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::WriteAsync(const void* data, size_t size, TransferCallback callback)
        {
            if (size == 0)
                return;
            
            while (!WriteReady()) ;
            _DmaTx::ClearTransferComplete();
            _DmaTx::SetTransferCallback(callback);
            _Regs()->CR3 |= USART_CR3_DMAT;
        #if defined (USART_TYPE_1)
            _Regs()->ICR |= TxCompleteInt;
        #endif
        #if defined (USART_TYPE_2)
            _Regs()->SR &= ~TxCompleteInt;
        #endif
            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->TRANSMIT_DATA_REG, size);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::Write(uint8_t data)
        {
            while (!WriteReady()) continue;

            _Regs()->TRANSMIT_DATA_REG = data;  
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::EnableInterrupt(InterruptFlags interruptFlags)
        {
            uint32_t cr1Mask = 0;
            uint32_t cr2Mask = 0;
            uint32_t cr3Mask = 0;

            if(interruptFlags & ParityErrorInt)
                cr1Mask |= USART_CR1_PEIE;

            static_assert(
                    USART_CR1_TXEIE  == TxEmptyInt &&
                    USART_CR1_TCIE   == TxCompleteInt &&
                    USART_CR1_RXNEIE == RxNotEmptyInt &&
                    USART_CR1_IDLEIE == IdleInt
                    );

            cr1Mask |= interruptFlags & (USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE);

        #if defined (USART_CR2_LBDIE)
            if(interruptFlags & LineBreakInt)
                cr2Mask |= USART_CR2_LBDIE;
        #endif

            if(interruptFlags & ErrorInt)
                cr3Mask |= USART_CR3_EIE;

            if(interruptFlags & CtsInt)
                cr3Mask |= USART_CR3_CTSIE;

            _Regs()->CR1 |= cr1Mask;
            _Regs()->CR2 |= cr2Mask;
            _Regs()->CR3 |= cr3Mask;

            if(interruptFlags != NoInterrupt)
                NVIC_EnableIRQ(_IRQNumber);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::DisableInterrupt(InterruptFlags interruptFlags)
        {
            uint32_t cr1Mask = 0;
            uint32_t cr2Mask = 0;
            uint32_t cr3Mask = 0;

            if(interruptFlags & ParityErrorInt)
                cr1Mask |= USART_CR1_PEIE;

            static_assert(
                    USART_CR1_TXEIE  == TxEmptyInt &&
                    USART_CR1_TCIE   == TxCompleteInt &&
                    USART_CR1_RXNEIE == RxNotEmptyInt &&
                    USART_CR1_IDLEIE == IdleInt
                    );

            cr1Mask |= interruptFlags & (USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE);

        #if defined (USART_CR2_LBDIE)
            if(interruptFlags & LineBreakInt)
                cr2Mask |= USART_CR2_LBDIE;
        #endif
            if(interruptFlags & ErrorInt)
                cr3Mask |= USART_CR3_EIE;

            if(interruptFlags & CtsInt)
                cr3Mask |= USART_CR3_CTSIE;

            _Regs()->CR1 &= ~cr1Mask;
            _Regs()->CR2 &= ~cr2Mask;
            _Regs()->CR3 &= ~cr3Mask;
        }

        USART_TEMPLATE_ARGS
        typename USART_TEMPLATE_QUALIFIER::InterruptFlags USART_TEMPLATE_QUALIFIER::InterruptSource()
        {
            return static_cast<InterruptFlags>(_Regs()->STATUS_REG & InterruptMask);
        }

        USART_TEMPLATE_ARGS
        typename USART_TEMPLATE_QUALIFIER::Error USART_TEMPLATE_QUALIFIER::GetError()
        {
            return static_cast<Error>(_Regs()->STATUS_REG & ErrorMask);
        }

        USART_TEMPLATE_ARGS
        void USART_TEMPLATE_QUALIFIER::ClearInterruptFlag(InterruptFlags interruptFlags)
        {
        #if defined(USART_TYPE_1)
            _Regs()->ICR |= interruptFlags;
        #endif
        #if defined(USART_TYPE_2)
            _Regs()->SR &= ~interruptFlags;
        #endif
        }
    }
}
#endif
