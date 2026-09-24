/**
 * @file
 * AXI DMA driver for NIIET K1921VG5T / K1921VG7T.
 *
 * This is *not* the PL230 µDMA of the K1921VG015 (see common/dma.h): here every
 * channel owns a full register block and executes a four-word *descriptor*
 * (SRC_PTR, DST_PTR, NDTL, CONFIG) that may chain to the next one. The driver
 * programs a single, non-chained descriptor (CMD_LAST) per Transfer(), which is
 * what the UART/SPI layers need, and exposes the same surface as the other
 * Zhele DMA back-ends so the portable peripheral code is unchanged.
 *
 * Hardware notes that shape the code (РП 12, A.4):
 *   - a channel is unclocked after reset and *ignores writes* until its bit in
 *     DMA->CH_ENABLE is set, so that comes first;
 *   - address increment is a static per-side bit (STATIC0.RD_INCR / STATIC1.WR_INCR),
 *     not an item-size field: set for memory, clear for a peripheral FIFO;
 *   - flow control comes from STATIC4.RD_PER_NUM / WR_PER_NUM — the peripheral
 *     request line number (see the DmaRequest table in the device header), 0 = memory;
 *   - NDTL.BUFFER_SIZE counts **bytes** and is 10 bits wide, so one descriptor
 *     moves at most 1023 bytes;
 *   - the channel raises its interrupt through CONFIG.CMD_SET_INT + INT_ENABLE.CH_END.
 *
 * @author Alexey Zhelonkin (NIIET port)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_DMA_AXI_H
#define ZHELE_PLATFORM_NIIET_COMMON_DMA_AXI_H

#include "ioreg.h"

extern "C" {
#include <plic.h>
}

#include <zhele/clock.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    class DmaBase
    {
    public:
        /**
         * @brief Transfer settings (combine with operator |).
         *
         * Direction picks which of Transfer()'s (buffer, periph) operands is the
         * source; the MSize / MemIncrement flags describe the memory side, the
         * PSize / PeriphIncrement flags the peripheral side.
         */
        enum Mode : uint32_t
        {
            Periph2Mem      = 0,        ///< Peripheral to memory
            Mem2Periph      = 1u << 0,  ///< Memory to peripheral
            Mem2Mem         = 1u << 1,  ///< Memory to memory (periph operand is the source)

            MemIncrement    = 1u << 2,  ///< Increment the memory address
            PeriphIncrement = 1u << 3,  ///< Increment the peripheral address

            MSize8Bits      = 0u << 4,  ///< Memory data size 8 bits
            MSize16Bits     = 1u << 4,  ///< Memory data size 16 bits
            MSize32Bits     = 2u << 4,  ///< Memory data size 32 bits

            PSize8Bits      = 0u << 6,  ///< Peripheral data size 8 bits
            PSize16Bits     = 1u << 6,  ///< Peripheral data size 16 bits
            PSize32Bits     = 2u << 6,  ///< Peripheral data size 32 bits

            PriorityHigh    = 1u << 8,  ///< Raise this channel in the read/write arbiters

            // A completed descriptor always raises CH_END here, so this is accepted
            // for API compatibility but needs no extra configuration bit.
            TransferCompleteInterrupt = 0,
        };

        /// Largest transfer one descriptor can carry (NDTL.BUFFER_SIZE is 10 bits).
        static constexpr uint32_t MaxTransferBytes = 1023;

    protected:
        static constexpr uint32_t MemSizeCode(uint32_t mode)    { return (mode >> 4) & 0x3u; }
        static constexpr uint32_t PeriphSizeCode(uint32_t mode) { return (mode >> 6) & 0x3u; }

        /// Bytes per item for a size code (0 → 1, 1 → 2, 2 → 4).
        static constexpr uint32_t ItemSize(uint32_t sizeCode) { return 1u << sizeCode; }

        /// AXI burst length used for the memory side of a memory-to-memory copy.
        static constexpr uint32_t MemBurstBytes = 16;
        /// Peripheral request latency (RD/WR_PER_DELAY); must be > 1 for peripherals.
        static constexpr uint32_t PeriphDelay = 2;
    };

    constexpr DmaBase::Mode operator|(DmaBase::Mode left, DmaBase::Mode right)
    {
        return static_cast<DmaBase::Mode>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
    }

    /**
     * @brief Per-channel bookkeeping for the completion callback.
     */
    struct DmaChannelData
    {
        using TransferCallback = std::add_pointer_t<void(void* data, unsigned size, bool success)>;

        TransferCallback transferCallback = nullptr; ///< User callback (optional)
        void* data = nullptr;                        ///< Memory buffer involved in the transfer
        uint16_t size = 0;                           ///< Transfer item count

        void NotifyTransferComplete()
        {
            if (transferCallback)
                transferCallback(data, size, true);
        }

        void NotifyError()
        {
            if (transferCallback)
                transferCallback(data, size, false);
        }
    };

    namespace Private
    {
        /**
         * @brief The DMA controller (one instance).
         *
         * @tparam _Regs      IO_STRUCT_WRAPPER over the DMA_TypeDef.
         * @tparam _Channels  Number of channels (8 on VG7T, 16 on VG5T).
         * @tparam _ClockCtrl Bus clock control for the controller.
         */
        template<typename _Regs, unsigned _Channels, typename _ClockCtrl>
        class DmaModule : public DmaBase
        {
        public:
            static constexpr unsigned Channels = _Channels;

            using Regs = _Regs;

            /// Clock the controller and the given channel (channels are gated individually).
            static void Enable(unsigned channel)
            {
                _ClockCtrl::Enable();
                _Regs()->CH_ENABLE |= (1u << channel);
            }

            static _DMA_CH_TypeDef& Channel(unsigned channel)
            {
                return _Regs()->CH[channel];
            }
        };

        /**
         * @brief One DMA channel, bound to a peripheral request line.
         *
         * Unlike the PL230 of the K1921VG015, channels here are interchangeable:
         * the peripheral is chosen by the request number written into STATIC4, so
         * the request is a template parameter of the *channel alias* a peripheral
         * is given (see the DmaRequest constants in the device header):
         *
         * @code
         * using UartTxDma = Zhele::Dma1Channel<0, Zhele::DmaRequest::Uart0>;
         * using Uart      = Zhele::Usart0<UartTxDma>;
         * @endcode
         *
         * @tparam _Module     Parent DmaModule.
         * @tparam _Channel    Hardware channel number.
         * @tparam _Request    Peripheral request line (0 = memory).
         * @tparam _IRQNumber  PLIC vector of this channel.
         */
        template<typename _Module, unsigned _Channel, uint32_t _Request, Plic_IsrVect_TypeDef _IRQNumber>
        class DmaChannel : public DmaBase
        {
            static_assert(_Channel < _Module::Channels);
            static inline DmaChannelData Data{};
            static inline bool IrqEnabled = false;

            /// Enable the channel's PLIC source once, like NVIC_EnableIRQ on STM32.
            static void EnableIrq()
            {
                if (IrqEnabled)
                    return;
                PLIC_SetPriority(_IRQNumber, 1);
                PLIC_SetMode(_IRQNumber, PLIC_IRQMODE_HILEVEL); // sources reset to OFF
                PLIC_IntEnable(Plic_Mach_Target, _IRQNumber);
                IrqEnabled = true;
            }

        public:
            using Module = _Module;
            using TransferCallback = DmaChannelData::TransferCallback;
            static constexpr unsigned Channel = _Channel;
            static constexpr uint32_t Request = _Request;
            static constexpr Plic_IsrVect_TypeDef IRQNumber = _IRQNumber;

            /**
             * @brief Arm a transfer. The peripheral's DMA request drives it to completion.
             *
             * @param [in] mode       Transfer settings (see Mode).
             * @param [in] buffer     Memory buffer.
             * @param [in] periph     Peripheral data register (or source memory for Mem2Mem).
             * @param [in] bufferSize Number of items to transfer.
             *
             * @note One descriptor carries at most MaxTransferBytes bytes; a larger
             *       request is rejected (nothing is started).
             */
            static void Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize)
            {
                if (bufferSize == 0)
                    return;

                const uint32_t memItem = ItemSize(MemSizeCode(mode));
                const uint32_t periphItem = ItemSize(PeriphSizeCode(mode));
                const uint32_t bytes = bufferSize * memItem;
                if (bytes > MaxTransferBytes)
                    return;

                _Module::Enable(_Channel);
                _DMA_CH_TypeDef& ch = _Module::Channel(_Channel);

                const bool memIsSource = (mode & Mem2Periph) != 0;
                const uint32_t memAddr = reinterpret_cast<uint32_t>(buffer);
                const uint32_t periphAddr = reinterpret_cast<uint32_t>(periph);
                // Mem2Mem takes the "periph" operand as a second memory buffer.
                const bool periphIsMemory = (mode & Mem2Mem) != 0;

                const uint32_t memBurst = periphIsMemory ? MemBurstBytes : periphItem;
                const uint32_t periphBurst = periphIsMemory ? MemBurstBytes : periphItem;
                const uint32_t memInc = (mode & MemIncrement) ? 1u : 0u;
                const uint32_t periphInc = (mode & PeriphIncrement) || periphIsMemory ? 1u : 0u;
                const uint32_t request = periphIsMemory ? 0u : _Request;

                uint32_t srcAddr, dstAddr, srcBurst, dstBurst, srcInc, dstInc, srcReq, dstReq;
                if (memIsSource)
                {
                    srcAddr = memAddr;    srcBurst = memBurst;    srcInc = memInc;    srcReq = 0;
                    dstAddr = periphAddr; dstBurst = periphBurst; dstInc = periphInc; dstReq = request;
                }
                else // Periph2Mem and Mem2Mem both read from the periph operand
                {
                    srcAddr = periphAddr; srcBurst = periphBurst; srcInc = periphInc; srcReq = request;
                    dstAddr = memAddr;    dstBurst = memBurst;    dstInc = memInc;    dstReq = 0;
                }

                ch.CH_ACTIVE = 0; // stop the channel while it is reprogrammed

                ch.STATIC0 = (srcBurst << DMA_CH_STATIC0_RD_BURST_MAX_Pos)
                           | (1u << DMA_CH_STATIC0_RD_TOKENS_Pos)
                           | (srcInc << DMA_CH_STATIC0_RD_INCR_Pos);
                ch.STATIC1 = (dstBurst << DMA_CH_STATIC1_WR_BURST_MAX_Pos)
                           | (1u << DMA_CH_STATIC1_WR_TOKENS_Pos)
                           | (dstInc << DMA_CH_STATIC1_WR_INCR_Pos);
                ch.STATIC2 = 0; // no joint mode, no endianness swap
                ch.STATIC3 = 0; // no arbitration wait limits
                ch.STATIC4 = (srcReq << DMA_CH_STATIC4_RD_PER_NUM_Pos)
                           | (srcReq ? (PeriphDelay << DMA_CH_STATIC4_RD_PER_DELAY_Pos) : 0u)
                           | (dstReq << DMA_CH_STATIC4_WR_PER_NUM_Pos)
                           | (dstReq ? (PeriphDelay << DMA_CH_STATIC4_WR_PER_DELAY_Pos) : 0u);

                const uint32_t mask = 1u << _Channel;
                if (mode & PriorityHigh)
                {
                    _Module::Regs::Get()->RD_PRIORITY |= mask;
                    _Module::Regs::Get()->WR_PRIORITY |= mask;
                }
                else
                {
                    _Module::Regs::Get()->RD_PRIORITY &= ~mask;
                    _Module::Regs::Get()->WR_PRIORITY &= ~mask;
                }

                ch.INT_CLEAR = 0xFFFFFFFFu;
                ch.INT_ENABLE = DMA_CH_INT_ENABLE_CH_END_Msk;

                // The descriptor itself: one buffer, no chaining, interrupt at the end.
                ch.SRC_PTR = srcAddr;
                ch.DST_PTR = dstAddr;
                ch.NDTL = bytes;
                ch.CONFIG = DMA_CH_CONFIG_CMD_SET_INT_Msk | DMA_CH_CONFIG_CMD_LAST_Msk;

                Data.data = const_cast<void*>(buffer);
                Data.size = static_cast<uint16_t>(bufferSize);
                if (Data.transferCallback != nullptr)
                    EnableIrq();

                ch.CH_ACTIVE = DMA_CH_CH_ACTIVE_CH_ACTIVE_Msk;
                ch.CH_START = DMA_CH_CH_START_CH_START_Msk;
            }

            static void SetTransferCallback(TransferCallback callback)
            {
                Data.transferCallback = callback;
            }

            /// True once the channel has stopped moving data.
            static bool Ready()
            {
                return (_Module::Channel(_Channel).CH_STATUS
                        & (DMA_CH_CH_STATUS_CH_RD_ACTIVE_Msk | DMA_CH_CH_STATUS_CH_WR_ACTIVE_Msk)) == 0;
            }

            static bool Enabled()
            {
                return (_Module::Channel(_Channel).CH_ACTIVE & DMA_CH_CH_ACTIVE_CH_ACTIVE_Msk) != 0;
            }

            /**
             * @brief Items still to be transferred.
             *
             * The controller only counts *completed buffers* (COUNT.BUFF_COUNT), not
             * bytes, so this reports the whole transfer while the channel is busy and
             * 0 once it has stopped — enough for "is it done", not a progress bar.
             */
            static uint32_t RemainingTransfers()
            {
                return Ready() ? 0u : Data.size;
            }

            static void Disable()
            {
                _Module::Channel(_Channel).CH_ACTIVE = 0;
            }

            /**
             * @brief Service this channel from its PLIC handler.
             *
             * Each channel has its own vector here (unlike the PL230), but the check
             * against CH_INTSTAT keeps it safe to call from a shared handler too.
             */
            static void IrqHandler()
            {
                const uint32_t mask = 1u << _Channel;
                if ((_Module::Regs::Get()->CH_INTSTAT & mask) == 0)
                    return;

                _DMA_CH_TypeDef& ch = _Module::Channel(_Channel);
                const uint32_t status = ch.INT_STATUS;
                ch.INT_CLEAR = status;

                if (status & ~DMA_CH_INT_STATUS_CH_END_Msk)
                    Data.NotifyError();
                else if (status & DMA_CH_INT_STATUS_CH_END_Msk)
                    Data.NotifyTransferComplete();
            }
        };
    } // namespace Private
} // namespace Zhele

#endif // ZHELE_PLATFORM_NIIET_COMMON_DMA_AXI_H
