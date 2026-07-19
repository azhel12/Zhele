/**
 * @file
 * DMA for CH32 (WCH DMA controller)
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_DMA_H
#define ZHELE_PLATFORM_CH32_COMMON_DMA_H

#include <zhele/common/template_utils/enum.h>

#include "ioreg.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    class DmaBase
    {
    public:
        /// DMA channel configuration flags (CFGR register bits).
        enum Mode : uint32_t
        {
            PriorityLow      = 0,                                    ///< Low priority
            PriorityMedium   = DMA_CFGR1_PL_0,                      ///< Medium priority
            PriorityHigh     = DMA_CFGR1_PL_1,                      ///< High priority
            PriorityVeryHigh = DMA_CFGR1_PL_0 | DMA_CFGR1_PL_1,     ///< Very high priority

            MSize8Bits  = 0,                  ///< Memory size 8 bits
            MSize16Bits = DMA_CFGR1_MSIZE_0,  ///< Memory size 16 bits
            MSize32Bits = DMA_CFGR1_MSIZE_1,  ///< Memory size 32 bits

            PSize8Bits  = 0,                  ///< Peripheral size 8 bits
            PSize16Bits = DMA_CFGR1_PSIZE_0,  ///< Peripheral size 16 bits
            PSize32Bits = DMA_CFGR1_PSIZE_1,  ///< Peripheral size 32 bits

            MemIncrement    = DMA_CFGR1_MINC,  ///< Enable memory increment
            PeriphIncrement = DMA_CFGR1_PINC,  ///< Enable peripheral increment
            Circular        = DMA_CFGR1_CIRC,  ///< Circular transfer

            Periph2Mem = 0,                    ///< Peripheral to memory
            Mem2Periph = DMA_CFGR1_DIR,        ///< Memory to peripheral
            Mem2Mem    = DMA_CFGR1_MEM2MEM,    ///< Memory to memory

            TransferErrorInterrupt    = DMA_CFGR1_TEIE,  ///< Transfer error interrupt
            HalfTransferInterrupt     = DMA_CFGR1_HTIE,  ///< Half transfer interrupt
            TransferCompleteInterrupt = DMA_CFGR1_TCIE   ///< Transfer complete interrupt
        };
    };

    /**
     * @brief Per-channel transfer state (user callback + buffer info).
     */
    struct DmaChannelData
    {
        using TransferCallback = std::add_pointer_t<void(void* data, unsigned size, bool success)>;

        DmaChannelData()
            : transferCallback(nullptr), data(nullptr), size(0)
        {}

        TransferCallback transferCallback; ///< Transfer complete/error callback
        void* data;                        ///< Data buffer
        uint16_t size;                     ///< Data buffer size

        void NotifyTransferComplete()
        {
            TransferCallback callback = transferCallback;
            if (callback != nullptr)
                callback(data, size, true);
        }

        void NotifyError()
        {
            TransferCallback callback = transferCallback;
            if (callback != nullptr)
                callback(data, size, false);
        }
    };

    /**
     * @brief Implements a DMA channel.
     *
     * @tparam _Module Parent DMA module
     * @tparam _ChannelRegs DMA_Channel_TypeDef wrapper
     * @tparam _Channel Channel number (1-based)
     * @tparam _IRQNumber Channel IRQ number
     */
    template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQNumber>
    class DmaChannel : public DmaBase
    {
        static_assert(_Channel >= 1 && _Channel <= _Module::Channels);

        // Flags for this channel sit in a 4-bit group at (4 * (channel - 1)).
        static constexpr uint32_t FlagsShift = 4u * (_Channel - 1);

        static DmaChannelData Data;
    public:
        using Module = _Module;
        using DmaBase::Mode;
        static constexpr unsigned Channel = _Channel;

        /**
         * @brief Configure the channel and start a transfer.
         *
         * @param [in] mode Channel mode (OR of Mode flags)
         * @param [in] buffer Memory buffer
         * @param [in] periph Peripheral address (or 2nd buffer for Mem2Mem)
         * @param [in] bufferSize Transfer length (in items)
         */
        static void Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize);

        static void SetTransferCallback(DmaChannelData::TransferCallback callback);

        static bool Ready();

        static bool Enabled();

        static void Enable();

        static void Disable();

        static uint32_t RemainingTransfers();

        static void* PeriphAddress();

        static void* MemAddress();

        static bool TransferError();

        static bool HalfTransfer();

        static bool TransferComplete();

        static void ClearFlags();

        static void ClearTransferError();

        static void ClearHalfTransfer();

        static void ClearTransferComplete();

        /**
         * @brief Channel IRQ handler — call from DMA1_ChannelN_IRQHandler().
         */
        static void IrqHandler();
    };

    /**
     * @brief Implements a DMA module (controller).
     *
     * @tparam _DmaRegs DMA_TypeDef wrapper
     * @tparam _Clock Source clock class
     * @tparam _Channels Channel count
     */
    template<typename _DmaRegs, typename _Clock, unsigned _Channels>
    class DmaModule : public DmaBase
    {
    public:
        static constexpr unsigned Channels = _Channels;

        static auto Regs();

        static void Enable();
        static void Disable();
    };
}

#include "impl/dma.h"

#endif // ZHELE_PLATFORM_CH32_COMMON_DMA_H
