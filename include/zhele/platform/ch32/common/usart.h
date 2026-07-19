/**
 * @file
 * USART for CH32
 * 
 * @author Aleksei Zhelonkin
 * 
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_USART_H
#define ZHELE_PLATFORM_CH32_COMMON_USART_H

#include <zhele/common/template_utils/enum.h>

#include <zhele/clock.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    class UsartBase
    {
    public:
        /**
         * @brief USART configuration (mirrors the STM32 UsartMode, WCH register names).
         */
        struct UsartMode
        {
            enum _CTLR1 : uint32_t
            {
                DataBits8 = 0,
                DataBits9 = USART_CTLR1_M,

                NoneParity = 0,
                EvenParity = USART_CTLR1_PCE,
                OddParity  = USART_CTLR1_PS | USART_CTLR1_PCE,

                Disabled   = 0,
                RxEnable   = USART_CTLR1_RE,
                TxEnable   = USART_CTLR1_TE,
                RxTxEnable = USART_CTLR1_RE | USART_CTLR1_TE,
                Default    = RxTxEnable,
            } CTLR1;

            enum _CTLR2 : uint32_t
            {
                OneStopBit         = 0,
                HalfStopBit        = USART_CTLR2_STOP_0,
                TwoStopBits        = USART_CTLR2_STOP_1,
                OneAndHalfStopBits = (USART_CTLR2_STOP_0 | USART_CTLR2_STOP_1),
            } CTLR2;

            enum _CTLR3 : uint32_t
            {
                FullDuplex = 0,
                HalfDuplex = USART_CTLR3_HDSEL,
            } CTLR3;

            constexpr UsartMode(UsartMode::_CTLR1 ctlr1) : CTLR1(ctlr1), CTLR2(OneStopBit), CTLR3(FullDuplex) {}
            constexpr UsartMode(UsartMode::_CTLR2 ctlr2) : CTLR1(Default), CTLR2(ctlr2), CTLR3(FullDuplex) {}
            constexpr UsartMode(UsartMode::_CTLR3 ctlr3) : CTLR1(Default), CTLR2(OneStopBit), CTLR3(ctlr3) {}

            constexpr UsartMode(UsartMode::_CTLR1 ctlr1, UsartMode::_CTLR2 ctlr2, UsartMode::_CTLR3 ctlr3) : CTLR1(ctlr1), CTLR2(ctlr2), CTLR3(ctlr3) {}

            constexpr UsartMode operator | (UsartMode::_CTLR1 flag)
            {
                return UsartMode{static_cast<UsartMode::_CTLR1>(CTLR1 | flag), CTLR2, CTLR3};
            }

            constexpr UsartMode operator | (UsartMode::_CTLR2 flag)
            {
                return UsartMode{CTLR1, static_cast<UsartMode::_CTLR2>(CTLR2 | flag), CTLR3};
            }

            constexpr UsartMode operator | (UsartMode::_CTLR3 flag)
            {
                return UsartMode{CTLR1, CTLR2, static_cast<UsartMode::_CTLR3>(CTLR3 | flag)};
            }
        };

        /// All possible interrupts (status bits live in STATR).
        enum InterruptFlags
        {
            NoInterrupt    = 0,
            ParityErrorInt = USART_STATR_PE,   ///< Parity error
            TxEmptyInt     = USART_STATR_TXE,  ///< Transmit data register empty
            TxCompleteInt  = USART_STATR_TC,   ///< Transmission complete
            RxNotEmptyInt  = USART_STATR_RXNE, ///< Read data register not empty
            IdleInt        = USART_STATR_IDLE, ///< Idle line detected
            LineBreakInt   = USART_STATR_LBD,  ///< LIN break detection
            ErrorInt       = USART_STATR_FE | USART_STATR_NE | USART_STATR_ORE,
            CtsInt         = USART_STATR_CTS,

            AllInterrupts  = ParityErrorInt | TxEmptyInt | TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt | ErrorInt | CtsInt,
        };

        enum Error
        {
            NoError      = 0,
            OverrunError = USART_STATR_ORE,
            NoiseError   = USART_STATR_NE,
            FramingError = USART_STATR_FE,
            ParityError  = USART_STATR_PE,
        };

    protected:
        static const unsigned ErrorMask = OverrunError | NoiseError | FramingError | ParityError;
        static const unsigned InterruptMask = ParityErrorInt | TxEmptyInt | TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt | ErrorInt | CtsInt;
    };

    static const UsartBase::UsartMode DefaultUsartMode = UsartBase::UsartMode::RxTxEnable;

    namespace Private
    {
        /**
         * @brief Implements the USART protocol.
         *
         * @tparam _Regs Peripheral register wrapper
         * @tparam _IRQNumber Global USART IRQ number
         * @tparam _ClockCtrl Clock control class
         * @tparam _RemapMask AFIO->PCFR1 mask of this USART's remap field
         * @tparam _RemapShift Bit offset of the remap field inside _RemapMask
         * @tparam _DmaTx TX DMA channel
         * @tparam _DmaRx RX DMA channel
         */
        template<
            typename _Regs,
            IRQn_Type _IRQNumber,
            typename _ClockCtrl,
            uint32_t _RemapMask,
            uint32_t _RemapShift,
            typename _DmaTx,
            typename _DmaRx>
        class Usart : public UsartBase
        {
        public:
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;
            using TransferCallback = DmaChannelData::TransferCallback;
            using Regs = _Regs;

            /// Initialize USART with a compile-time baud rate.
            template<unsigned long baud>
            static void Init(UsartMode mode = DefaultUsartMode);

            /// Initialize USART.
            static void Init(unsigned baud, UsartMode mode = DefaultUsartMode);

            static void SetConfig(UsartMode modeMask);

            static void ClearConfig(UsartMode modeMask);

            static void SetBaud(unsigned baud);

            static bool ReadReady();

            static uint8_t Read();

            /**
             * @brief Enable async read (by DMA).
             *
             * @note The RX DMA channel IRQ must route to DmaRx::IrqHandler().
             */
            static void EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);

            static bool WriteReady();

            /// Write data to USART (blocking).
            static void Write(const void* data, size_t size);

            /**
             * @brief Write data to USART async (via DMA).
             *
             * @note The TX DMA channel IRQ must route to DmaTx::IrqHandler().
             */
            static void WriteAsync(const void* data, size_t size, TransferCallback callback = nullptr);

            /// Synchronous write of a single byte.
            static void Write(uint8_t data);

            static void EnableInterrupt(InterruptFlags interruptFlags);

            static void DisableInterrupt(InterruptFlags interruptFlags);

            static InterruptFlags InterruptSource();

            static Error GetError();

            /// Clear the given status flags (WCH STATR is cleared by writing 0 to the bit).
            static void ClearInterruptFlag(InterruptFlags interruptFlags);

            static void ClearAllInterruptFlags();

            /**
             * @brief Select TX/RX pins and apply the global AFIO remap (0 = default pins).
             */
            template<typename TxPin, typename RxPin, uint8_t Remap = 0>
            static void SelectTxRxPins();
        };
    }
}

#include "impl/usart.h"

#endif // ZHELE_PLATFORM_CH32_COMMON_USART_H
