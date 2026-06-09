/**
 * @file
 * Portable µDMA (PL230) driver for NIIET K1921VG015.
 *
 * @author Alexey Zhelonkin (NIIET port)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_DMA_H
#define ZHELE_PLATFORM_NIIET_COMMON_DMA_H

#include "ioreg.h"

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
         * PSize / PeriphIncrement flags the peripheral side. The driver maps those
         * onto the hardware source/destination fields according to direction.
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

            PriorityHigh    = 1u << 8,  ///< Use the high-priority channel set

            // The µDMA always raises a done interrupt for a completed channel, so
            // these are accepted for API compatibility but need no config bits.
            TransferCompleteInterrupt = 0,
        };

    protected:
        // Hardware increment encoding for the "no increment" case.
        static constexpr uint32_t IncNone = 3u;

        // CHANNEL_CFG.CYCLE_CTRL value for a basic peripheral/memory cycle.
        static constexpr uint32_t CycleBasic = 1u;

        static constexpr uint32_t MemSizeCode(uint32_t mode)    { return (mode >> 4) & 0x3u; }
        static constexpr uint32_t PeriphSizeCode(uint32_t mode) { return (mode >> 6) & 0x3u; }

        // End pointer = address of the last item the controller accesses.
        static constexpr uint32_t EndPtr(uint32_t start, uint32_t incCode, uint32_t count)
        {
            return incCode == IncNone ? start : start + ((count - 1) << incCode);
        }
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

    /**
     * @brief DMA controller (the single µDMA instance + its control table).
     *
     * @tparam _Regs  IO_STRUCT_WRAPPER over the DMA_TypeDef.
     * @tparam _Table Type exposing `static DMA_CtrlData_TypeDef& Get()` — the
     *                1 KB-aligned control table in RAM.
     */
    template<typename _Regs, typename _Table>
    class DmaModule : public DmaBase
    {
    public:
        static constexpr unsigned Channels = 24;

        using Regs = _Regs;

        /// Point the controller at the control table and enable it (idempotent).
        static void Enable()
        {
            if ((_Regs()->STATUS & DMA_STATUS_MASTEREN_Msk) != 0)
                return;
            _Regs()->BASEPTR = reinterpret_cast<uint32_t>(&_Table::Get());
            _Regs()->CFG = DMA_CFG_MASTEREN_Msk;
        }

        /// Primary control descriptor for a channel.
        static DMA_Channel_TypeDef& Channel(unsigned channel)
        {
            return _Table::Get().PRM_DATA.CH[channel];
        }
    };

    /**
     * @brief A single DMA channel bound to a fixed hardware channel number.
     *
     * @tparam _Module     Parent DmaModule.
     * @tparam _Channel    Hardware channel number (0..23, fixed per peripheral).
     * @tparam _IRQNumber  Shared DMA interrupt vector this channel belongs to.
     */
    template<typename _Module, unsigned _Channel, Plic_IsrVect_TypeDef _IRQNumber>
    class DmaChannel : public DmaBase
    {
        static_assert(_Channel < _Module::Channels);
        static inline DmaChannelData Data{};

    public:
        using Module = _Module;
        using TransferCallback = DmaChannelData::TransferCallback;
        static constexpr unsigned Channel = _Channel;
        static constexpr Plic_IsrVect_TypeDef IRQNumber = _IRQNumber;

        /**
         * @brief Arm a transfer. The peripheral's DMA request drives it to completion.
         *
         * @param [in] mode       Transfer settings (see Mode).
         * @param [in] buffer     Memory buffer.
         * @param [in] periph     Peripheral data register (or source memory for Mem2Mem).
         * @param [in] bufferSize Number of items to transfer.
         */
        static void Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize)
        {
            if (bufferSize == 0)
                return;

            _Module::Enable();

            const uint32_t memSize = MemSizeCode(mode);
            const uint32_t periphSize = PeriphSizeCode(mode);
            const uint32_t memInc = (mode & MemIncrement) ? memSize : IncNone;
            const uint32_t periphInc = (mode & PeriphIncrement) ? periphSize : IncNone;

            const uint32_t memAddr = reinterpret_cast<uint32_t>(buffer);
            const uint32_t periphAddr = reinterpret_cast<uint32_t>(periph);

            uint32_t srcAddr, dstAddr, srcSize, dstSize, srcInc, dstInc;
            if (mode & Mem2Periph)
            {
                srcAddr = memAddr;    srcSize = memSize;    srcInc = memInc;
                dstAddr = periphAddr; dstSize = periphSize; dstInc = periphInc;
            }
            else // Periph2Mem and Mem2Mem both take periph as the source
            {
                srcAddr = periphAddr; srcSize = periphSize; srcInc = periphInc;
                dstAddr = memAddr;    dstSize = memSize;    dstInc = memInc;
            }

            DMA_Channel_TypeDef& ch = _Module::Channel(_Channel);
            ch.SRC_DATA_END_PTR = EndPtr(srcAddr, srcInc, bufferSize);
            ch.DST_DATA_END_PTR = EndPtr(dstAddr, dstInc, bufferSize);
            ch.CHANNEL_CFG =
                  (CycleBasic << DMA_CHANNEL_CFG_CYCLE_CTRL_Pos)
                | ((bufferSize - 1) << DMA_CHANNEL_CFG_N_MINUS_1_Pos)
                | (srcInc  << DMA_CHANNEL_CFG_SRC_INC_Pos)
                | (srcSize << DMA_CHANNEL_CFG_SRC_SIZE_Pos)
                | (dstInc  << DMA_CHANNEL_CFG_DST_INC_Pos)
                | (dstSize << DMA_CHANNEL_CFG_DST_SIZE_Pos);

            Data.data = const_cast<void*>(buffer);
            Data.size = static_cast<uint16_t>(bufferSize);

            const uint32_t mask = 1u << _Channel;
            _Module::Regs::Get()->PRIALTCLR = mask;  // use the primary structure
            _Module::Regs::Get()->REQMASKCLR = mask; // allow the peripheral to request
            if (mode & PriorityHigh)
                _Module::Regs::Get()->PRIORITYSET = mask;
            else
                _Module::Regs::Get()->PRIORITYCLR = mask;
            _Module::Regs::Get()->ENSET = mask;      // arm the channel
        }

        static void SetTransferCallback(TransferCallback callback)
        {
            Data.transferCallback = callback;
        }

        /// True once the transfer has finished (controller cleared the enable bit).
        static bool Ready()
        {
            return (_Module::Regs::Get()->ENSET & (1u << _Channel)) == 0;
        }

        static bool Enabled()
        {
            return (_Module::Regs::Get()->ENSET & (1u << _Channel)) != 0;
        }

        /// Items still to be transferred (0 once the cycle has stopped).
        static uint32_t RemainingTransfers()
        {
            const uint32_t cfg = _Module::Channel(_Channel).CHANNEL_CFG;
            if ((cfg & DMA_CHANNEL_CFG_CYCLE_CTRL_Msk) == 0)
                return 0;
            return ((cfg & DMA_CHANNEL_CFG_N_MINUS_1_Msk) >> DMA_CHANNEL_CFG_N_MINUS_1_Pos) + 1;
        }

        static void Disable()
        {
            _Module::Regs::Get()->ENCLR = 1u << _Channel;
        }

        /**
         * @brief Service this channel from the shared DMA ISR.
         *
         * Several channels share one interrupt vector, so this only acts if its own
         * channel's done bit is set. Call it for each candidate channel from the ISR.
         */
        static void IrqHandler()
        {
            const uint32_t mask = 1u << _Channel;
            if ((_Module::Regs::Get()->IRQSTAT & mask) == 0)
                return;
            _Module::Regs::Get()->IRQSTATCLR = mask;
            Data.NotifyTransferComplete();
        }
    };
}

#endif // ZHELE_PLATFORM_NIIET_COMMON_DMA_H
