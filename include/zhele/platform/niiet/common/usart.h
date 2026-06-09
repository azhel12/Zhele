/**
 * @file
 * UART driver for NIIET K1921VG015.
 *
 * @author Alexey Zhelonkin (NIIET port)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_USART_H
#define ZHELE_PLATFORM_NIIET_COMMON_USART_H

#include "ioreg.h"

#include <zhele/clock.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>
#include <zhele/pinlist.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    class UsartBase
    {
    public:
        /**
         * @brief Line format (LCRH) + transfer enable (CR) settings.
         *
         * Compose with operator |, e.g. `UsartMode::DataBits8 | UsartMode::EvenParity`.
         */
        struct UsartMode
        {
            /// Line control: word length, parity, stop bits, FIFO.
            enum _LCRH : uint32_t
            {
                DataBits5 = 0u << UART_LCRH_WLEN_Pos,
                DataBits6 = 1u << UART_LCRH_WLEN_Pos,
                DataBits7 = 2u << UART_LCRH_WLEN_Pos,
                DataBits8 = 3u << UART_LCRH_WLEN_Pos,

                NoneParity = 0,
                EvenParity = UART_LCRH_PEN_Msk | UART_LCRH_EPS_Msk,
                OddParity  = UART_LCRH_PEN_Msk,

                OneStopBit  = 0,
                TwoStopBits = UART_LCRH_STP2_Msk,

                FifoDisable = 0,
                FifoEnable  = UART_LCRH_FEN_Msk,

                DefaultFormat = DataBits8 | FifoEnable,
            } LCRH;

            /// Control: receiver / transmitter enable.
            enum _CR : uint32_t
            {
                Disabled   = 0,
                RxEnable   = UART_CR_RXE_Msk,
                TxEnable   = UART_CR_TXE_Msk,
                RxTxEnable = UART_CR_RXE_Msk | UART_CR_TXE_Msk,
                Default    = RxTxEnable,
            } CR;

            constexpr UsartMode(_LCRH lcrh) : LCRH(lcrh), CR(Default) {}
            constexpr UsartMode(_CR cr) : LCRH(DefaultFormat), CR(cr) {}
            constexpr UsartMode(_LCRH lcrh, _CR cr) : LCRH(lcrh), CR(cr) {}

            // Combine flags within a register strongly-typed (keeps `A | B` an
            // _LCRH/_CR instead of decaying to the built-in int operator).
            friend constexpr _LCRH operator|(_LCRH left, _LCRH right)
            {
                return static_cast<_LCRH>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
            }

            friend constexpr _CR operator|(_CR left, _CR right)
            {
                return static_cast<_CR>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
            }

            constexpr UsartMode operator|(_LCRH flag) const
            {
                return UsartMode{static_cast<_LCRH>(LCRH | flag), CR};
            }

            constexpr UsartMode operator|(_CR flag) const
            {
                return UsartMode{LCRH, static_cast<_CR>(CR | flag)};
            }
        };

        /**
         * @brief Interrupt sources (shared bit layout across IMSC/RIS/MIS/ICR).
         */
        enum InterruptFlags : uint32_t
        {
            NoInterrupt     = 0,
            RxNotEmptyInt   = UART_IMSC_RXIM_Msk,  ///< Receive (FIFO past threshold)
            TxEmptyInt      = UART_IMSC_TXIM_Msk,  ///< Transmit (FIFO past threshold)
            ReceiveTimeout  = UART_IMSC_RTIM_Msk,  ///< Receive timeout (idle line)
            FramingErrorInt = UART_IMSC_FERIM_Msk,
            ParityErrorInt  = UART_IMSC_PERIM_Msk,
            BreakErrorInt   = UART_IMSC_BERIM_Msk,
            OverrunErrorInt = UART_IMSC_OERIM_Msk,
            TxCompleteInt   = UART_IMSC_TDIM_Msk,  ///< Transmit done (line idle after TX)

            ErrorInt = FramingErrorInt | ParityErrorInt | BreakErrorInt | OverrunErrorInt,
            AllInterrupts = RxNotEmptyInt | TxEmptyInt | ReceiveTimeout | ErrorInt | TxCompleteInt,
        };

        /**
         * @brief Receive errors (RSR register).
         */
        enum Error : uint32_t
        {
            NoError      = 0,
            FramingError = UART_RSR_FE_Msk,
            ParityError  = UART_RSR_PE_Msk,
            BreakError   = UART_RSR_BE_Msk,
            OverrunError = UART_RSR_OE_Msk,
        };

    protected:
        static constexpr uint32_t ErrorMask =
            UART_RSR_FE_Msk | UART_RSR_PE_Msk | UART_RSR_BE_Msk | UART_RSR_OE_Msk;
    };

    static constexpr UsartBase::UsartMode DefaultUsartMode{UsartBase::UsartMode::DefaultFormat,
                                                           UsartBase::UsartMode::Default};

    namespace Private
    {
        /**
         * @brief PL011 UART instance.
         *
         * @tparam _Regs       IO_STRUCT_WRAPPER over the UART_TypeDef.
         * @tparam _IRQNumber  PLIC vector for this UART.
         * @tparam _ClockCtrl  Clock control (provides Enable/Disable/ClockFreq).
         * @tparam _TxPins     Pin map struct (io_pins PinList + alt_functions array).
         * @tparam _RxPins     Pin map struct.
         * @tparam _DmaTx      DMA TX channel (or void to disable async write).
         * @tparam _DmaRx      DMA RX channel (or void to disable async read).
         */
        template<typename _Regs, Plic_IsrVect_TypeDef _IRQNumber, typename _ClockCtrl,
                 typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        class Usart : public UsartBase
        {
        public:
            using Regs = _Regs;
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;
            using TransferCallback = DmaChannelData::TransferCallback;

            static constexpr Plic_IsrVect_TypeDef IRQNumber = _IRQNumber;

            template<unsigned long baud>
            static void Init(UsartMode mode = DefaultUsartMode)
            {
                Init(baud, mode);
            }

            static void Init(unsigned baud, UsartMode mode = DefaultUsartMode)
            {
                _ClockCtrl::Enable();
                _Regs()->CR = 0;                                   // disable while reconfiguring
                SetDivisors(baud);
                _Regs()->LCRH = static_cast<uint32_t>(mode.LCRH);  // also latches the divisors
                _Regs()->CR = static_cast<uint32_t>(mode.CR) | UART_CR_UARTEN_Msk;
            }

            static void SetBaud(unsigned baud)
            {
                SetDivisors(baud);
                _Regs()->LCRH = _Regs()->LCRH; // re-write to latch the new divisors
            }

            static bool WriteReady()
            {
                return (_Regs()->FR & UART_FR_TXFF_Msk) == 0;
            }

            static void Write(uint8_t data)
            {
                while (!WriteReady()) { }
                _Regs()->DR = data;
            }

            static void Write(const void* data, size_t size)
            {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                for (size_t i = 0; i < size; ++i)
                    Write(bytes[i]);
            }

            static bool ReadReady()
            {
                return (_Regs()->FR & UART_FR_RXFE_Msk) == 0;
            }

            static uint8_t Read()
            {
                while (!ReadReady()) { }
                return static_cast<uint8_t>(_Regs()->DR & UART_DR_DATA_Msk);
            }

            static void WriteAsync(const void* data, size_t size, TransferCallback callback = nullptr)
            {
                if constexpr (!std::is_same_v<_DmaTx, void>)
                {
                    _DmaTx::SetTransferCallback(callback);
                    _Regs()->DMACR |= UART_DMACR_TXDMAE_Msk;
                    _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | _DmaTx::MSize8Bits | _DmaTx::PSize8Bits,
                                     data, &_Regs()->DR, size);
                }
            }

            static void EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr)
            {
                if constexpr (!std::is_same_v<_DmaRx, void>)
                {
                    _DmaRx::SetTransferCallback(callback);
                    _Regs()->DMACR |= UART_DMACR_RXDMAE_Msk;
                    _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::MSize8Bits | _DmaRx::PSize8Bits,
                                     receiveBuffer, &_Regs()->DR, bufferSize);
                }
            }

            static void EnableInterrupt(InterruptFlags interruptFlags)
            {
                _Regs()->IMSC |= static_cast<uint32_t>(interruptFlags);
            }

            static void DisableInterrupt(InterruptFlags interruptFlags)
            {
                _Regs()->IMSC &= ~static_cast<uint32_t>(interruptFlags);
            }

            static InterruptFlags InterruptSource()
            {
                return static_cast<InterruptFlags>(_Regs()->MIS);
            }

            static void ClearInterruptFlag(InterruptFlags interruptFlags)
            {
                _Regs()->ICR = static_cast<uint32_t>(interruptFlags);
            }

            static Error GetError()
            {
                return static_cast<Error>(_Regs()->RSR & ErrorMask);
            }

            static void SelectTxRxPins(int8_t txPinNumber, int8_t rxPinNumber = -1)
            {
                using TxPins = typename _TxPins::io_pins;
                using RxPins = typename _RxPins::io_pins;
                using Type = typename TxPins::DataType;

                TxPins::Enable();
                Type maskTx(1 << txPinNumber);
                TxPins::SetConfiguration(TxPins::Configuration::AltFunc, maskTx);
                TxPins::AltFuncNumber(_TxPins::alt_functions[static_cast<size_t>(txPinNumber)], maskTx);

                if (rxPinNumber != -1)
                {
                    RxPins::Enable();
                    Type maskRx(1 << rxPinNumber);
                    RxPins::SetConfiguration(RxPins::Configuration::AltFunc, maskRx);
                    RxPins::AltFuncNumber(_RxPins::alt_functions[static_cast<size_t>(rxPinNumber)], maskRx);
                }
            }

            template<int8_t TxPinNumber, int8_t RxPinNumber = -1>
            static void SelectTxRxPins()
            {
                using TxPin = typename _TxPins::io_pins::template Pin<TxPinNumber>;

                TxPin::Port::Enable();
                TxPin::template SetConfiguration<TxPin::Port::Configuration::AltFunc>();
                TxPin::template AltFuncNumber<_TxPins::alt_functions[TxPinNumber]>();

                if constexpr (RxPinNumber != -1)
                {
                    using RxPin = typename _RxPins::io_pins::template Pin<RxPinNumber>;

                    if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>)
                        RxPin::Port::Enable();

                    RxPin::template SetConfiguration<RxPin::Port::Configuration::AltFunc>();
                    RxPin::template AltFuncNumber<_RxPins::alt_functions[RxPinNumber]>();
                }
            }

            template<typename TxPin, typename RxPin = typename IO::NullPin>
            static void SelectTxRxPins()
            {
                const int8_t txPinIndex = _TxPins::io_pins::template IndexOf<TxPin>;
                const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                              ? _RxPins::io_pins::template IndexOf<RxPin>
                                              : -1;
                static_assert(txPinIndex >= 0);
                static_assert(rxPinIndex >= -1);
                SelectTxRxPins<txPinIndex, rxPinIndex>();
            }

        private:
            static void SetDivisors(unsigned baud)
            {
                const uint32_t clk = _ClockCtrl::ClockFreq();
                const uint32_t denom = 16u * baud;
                const uint32_t integer = clk / denom;
                // Fractional part rounded to 1/64: round((clk%denom)/denom * 64).
                const uint32_t fraction = ((clk % denom) * 64u + denom / 2u) / denom;
                _Regs()->IBRD = integer;
                _Regs()->FBRD = fraction;
            }
        };
    } // namespace Private
} // namespace Zhele

#endif // ZHELE_PLATFORM_NIIET_COMMON_USART_H
