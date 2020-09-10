/**
 * @file
 * Implement UART protocol
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_UART_COMMON_H
#define ZHELE_UART_COMMON_H

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "ioreg.h"
#include "pinlist.h"

namespace Zhele
{
#if(USART_SR_PE)
    #define FOR_SR(TEXT) TEXT
    #define FOR_ISR(TEXT)
#endif
#if(USART_ISR_PE)
    #define FOR_SR(TEXT)
    #define FOR_ISR(TEXT) TEXT
#endif
    class UsartBase
    {
    public:
        /// Usart modes
        enum UsartMode
        {
            DataBits8 = 0,
            DataBits9 = USART_CR1_M,

            NoneParity = 0,
            EvenParity = USART_CR1_PCE,
            OddParity  = USART_CR1_PS | USART_CR1_PCE,

            NoClock = 0,

            Disabled = 0,
            RxEnable = USART_CR1_RE,
            TxEnable = USART_CR1_TE,
            RxTxEnable  = USART_CR1_RE | USART_CR1_TE,
            Default = RxTxEnable,

            OneStopBit         = 0,
            HalfStopBit        = USART_CR2_STOP_0 << 16,
            TwoStopBits        = USART_CR2_STOP_1 << 16,
            OneAndHalfStopBits = (USART_CR2_STOP_0 | USART_CR2_STOP_1) << 16
        };

        /**
         * @brief All possible interrupts
         */
        enum InterruptFlags
        {
            NoInterrupt = 0,
        #if defined (USART_SR_PE)
            ParityErrorInt = USART_SR_PE,	//< Parity error
            TxEmptyInt     = USART_SR_TXE,	//< Transmission register empty
            TxCompleteInt  = USART_SR_TC,	//< Transmission complete
            RxNotEmptyInt  = USART_SR_RXNE,	//< Read data register not empty
            IdleInt        = USART_SR_IDLE,	//< Idle mode (all operations finished)

            LineBreakInt   = USART_SR_LBD,

            ErrorInt       = USART_SR_FE | USART_SR_NE | USART_SR_ORE,	
            CtsInt         = USART_SR_CTS,
        #endif
        #if defined (USART_ISR_PE)
            ParityErrorInt = USART_ISR_PE,	//< Parity error
            TxEmptyInt     = USART_ISR_TXE,	//< Transmission register empty
            TxCompleteInt  = USART_ISR_TC,	//< Transmission complete
            RxNotEmptyInt  = USART_ISR_RXNE,	//< Read data register not empty
            IdleInt        = USART_ISR_IDLE,	//< Idle mode (all operations finished)
            #if defined(USART_ISR_LBD)
            LineBreakInt   = USART_ISR_LBD,
            #else
            LineBreakInt   = 0,
            #endif
            ErrorInt       = USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE,	
            CtsInt         = USART_ISR_CTS,
        #endif
            AllInterrupts  = ParityErrorInt | TxEmptyInt | TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt | ErrorInt | CtsInt
        };

        enum Error
        {
            NoError = 0,
        #if defined (USART_SR_PE)
            OverrunError = USART_SR_ORE,
            NoiseError = USART_SR_NE,
            FramingError = USART_SR_FE,
            ParityError = USART_SR_PE
        #endif
        #if defined (USART_ISR_PE)
            OverrunError = USART_ISR_ORE,
            NoiseError = USART_ISR_NE,
            FramingError = USART_ISR_FE,
            ParityError = USART_ISR_PE
        #endif
        };

    protected:
        static const unsigned ErrorMask = OverrunError | NoiseError | FramingError | ParityError;

        static const unsigned InterruptMask = ParityErrorInt | TxEmptyInt |
                TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt |
                ErrorInt | CtsInt;

        static const unsigned CR1ModeMask =
            USART_CR1_M |
            USART_CR1_PCE |
            USART_CR1_PS |
            USART_CR1_RE |
            USART_CR1_TE;

        static const unsigned CR2ModeMask = USART_CR2_STOP_0 | USART_CR2_STOP_1;
        enum
        {
            CR1ModeShift = 0,
            CR2ModeShift = 16
        };
    };

    inline UsartBase::UsartMode operator|(UsartBase::UsartMode left, UsartBase::UsartMode right)
    {	return static_cast<UsartBase::UsartMode>(static_cast<unsigned>(left) | static_cast<unsigned>(right));	}

    inline UsartBase::InterruptFlags operator|(UsartBase::InterruptFlags left, UsartBase::InterruptFlags right)
    {	return static_cast<UsartBase::InterruptFlags>(static_cast<unsigned>(left) | static_cast<unsigned>(right));	}

    inline UsartBase::Error operator|(UsartBase::Error left, UsartBase::Error right)
    {	return static_cast<UsartBase::Error>(static_cast<unsigned>(left) | static_cast<unsigned>(right));	}


    namespace Private
    {
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        class Usart : public UsartBase
        {
        public:
            /**
             * @brief Initialize USART
             * 
             * @tparam baud Baud rate
             * @param [in] usartMode Mode
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned long baud>
            static inline void Init(UsartMode usartMode = Default)
            {
                Init(baud, usartMode);
            }

            /**
             * @brief Initialize USART
             * 
             * @param [in] baud Baud rate
             * @param[in] usartMode Mode
             * 
             * @par Returns
             *	Nothing
             */
            static void Init(unsigned baud, UsartMode usartMode = Default)
            {
                _ClockCtrl::Enable();
                unsigned brr = _ClockCtrl::ClockFreq() / baud;
                _Regs()->BRR = brr;
                _Regs()->FOR_SR(SR)FOR_ISR(ISR) = 0x00;
                _Regs()->CR2 = ((usartMode >> CR2ModeShift) & CR2ModeMask);
                _Regs()->CR1 = (((usartMode >> CR1ModeShift) & CR1ModeMask) | USART_CR1_UE );
            }

            /**
             * @brief Check that USART ready to read
             * 
             * @retval true USART ready for read
             * @retval false USART is not ready for read
             */
            static bool ReadReady()
            {
                return _Regs()->FOR_SR(SR)FOR_ISR(ISR) & RxNotEmptyInt;
            }

            /**
             * @brief Synch read data
             * 
             * @returns Readed byte
             */
            static uint8_t Read()
            {
                while(!ReadReady())
                    ;
                return _Regs()->FOR_SR(DR)FOR_ISR(RDR);
            }

            /**
             * @brief Enable async read (by DMA)
             * 
             * @param [out] receiveBuffer Output buffer
             * @param [in] bufferSize Output buffer size
             * @param [in] callback Transfer complete callback (optional parameter)
             * 
             * @par Returns
             * 	Nothing
             */
            static void EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr)
            {
                _DmaRx::ClearTransferComplete();
                _Regs()->CR3 |= USART_CR3_DMAR;
                _DmaRx::SetTransferCallback(callback);
                _DmaRx::Init(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, receiveBuffer, &_Regs()->DR, bufferSize);
            }

            /**
             * @brief Check that USART ready to write
             * 
             * @retval true USART ready for write
             * @retval false USART is not ready for write
             */
            static bool WriteReady()
            {
                bool dmaActive = (_Regs()->CR3 & USART_CR3_DMAT) && _DmaTx::Enabled();
                return (!dmaActive || _DmaTx::TransferComplete()) && (_Regs()->FOR_SR(SR)FOR_ISR(ISR) & TxEmptyInt);
            }

            /**
             * @brief Write data to USART
             * 
             * @param [in] data Data to write
             * @param [in] size Data size
             * @param [in] async Write async or no
             * 
             * @par Returns
             * 	Nothing
             */
            static void Write(const void* data, size_t size, bool async = false)
            {
                if (async && size > 1)
                {
                    while (!WriteReady()) ;
                    _DmaTx::ClearTransferComplete();
                    _Regs()->CR3 |= USART_CR3_DMAT;
                    _Regs()->FOR_SR(SR)FOR_ISR(ISR) &= ~TxCompleteInt;
                    _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->FOR_SR(DR)FOR_ISR(TDR), size);
                }
                else
                {
                    uint8_t *ptr = static_cast<uint8_t*>(const_cast<void*>(data));
                    while (size--)
                    {
                        Write(*ptr++);
                    }
                }
            }

            /**
             * @brief Synch write byte
             * 
             * @param [in] data Byte to write
             * 
             * @par Returns
             *	Nothing
             */
            static void Write(uint8_t data)
            {
                while (!WriteReady()) ;

                _Regs()->FOR_SR(DR)FOR_ISR(TDR) = data;
            }

            /**
             * @brief Enables one or more interrupts
             * 
             * @param [in] interruptFlags Interrupt flags (supports OR ("||") logic operation)
             * 
             * @par Returns
             *	Nothing
             */
            static void EnableInterrupt(InterruptFlags interruptFlags)
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

            /**
             * @brief Disables one or more interrupts
             * 
             * @param [in] interruptFlags Interrupt flags (supports OR ("||") logic operation)
             * 
             * @par Returns
             *	Nothing
             */
            static void DisableInterrupt(InterruptFlags interruptFlags)
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

            /**
             * @brief Returns caused interrupts
             * 
             * @returns Mask of interrupts
             */
            static InterruptFlags InterruptSource()
            {
                return static_cast<InterruptFlags>(_Regs()->FOR_SR(SR)FOR_ISR(ISR) & InterruptMask);
            }

            /**
             * @brief Returns caused errors
             * 
             * @returns Mask of errors
             */
            static Error GetError()
            {
                return static_cast<Error>(_Regs()->FOR_SR(SR)FOR_ISR(ISR) & ErrorMask);
            }

            /**
             * @brief Clears interrupts
             * 
             * @param [in] interruptFlags Interrupts mask
             * 
             * @par Returns
             *	Nothing
             */
            static void ClearInterruptFlag(InterruptFlags interruptFlags)
            {
                _Regs()->FOR_SR(SR)FOR_ISR(ICR) &= ~interruptFlags;
            }

            /**
             * @brief Select RX and TX pins (set settings)
             * 
             * @param [in] txPinNumber pin number in Txs PinList
             * @param [in] rxPinNumber pin number in Rxs PinList
             * 
             * @par Returns
             *	Nothing
             */
            static void SelectTxRxPins(uint8_t txPinNumber, uint8_t rxPinNumber);

            /**
             * @brief Select RX and TX pins (set settings)
             * 
             * @param [in] txPinNumber pin number in Txs PinList
             * @param [in] rxPinNumber pin number in Rxs PinList
             * 
             * @par Returns
             *	Nothing
             */
            static void SelectTxRxPins(uint8_t txPinNumber, uint8_t rxPinNumber, uint8_t txPinAltFuncNumber, uint8_t rxPinAltFuncNumber);

            /**
             * @brief Template clone of SelectTxRxPins method
             * 
             * @tparam TxPinNumber pin number in Txs PinList
             * @tparam RxPinNumber pin number in Rxs PinList
             * 
             * @par Returns
             *	Nothing
             */
            template<uint8_t TxPinNumber, uint8_t RxPinNumber>
            static void SelectTxRxPins();

            /**
             * @brief Template clone of SelectTxRxPins method (params are TPin instances)
             * 
             * @tparam TxPin TxPin
             * @tparam RxPin RxPin
             * 
             * @par Returns
             *	Nothing
             */
            template<typename TxPin, typename RxPin>
            static void SelectTxRxPins();
        };
    }
}
#endif