/**
 * @file
 * Implement DMA (GPDMA) for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_DMA_H
#define ZHELE_PLATFORM_STM32_H5_DMA_H

#include <stm32h5xx.h>

#include <zhele/common/template_utils/enum.h>
#include "../common/ioreg.h"

#include "clock.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    class DmaBase
    {
    public:
        /**
         * @brief DMA settings
         *
         * @details
         * This is *not* a register layout: GPDMA has no single control register
         * with these fields. Transfer() decodes it into CxTR1/CxTR2/CxCR.
         */
        enum Mode : uint32_t
        {
            PriorityLow = 0u << 0,          ///< Low priority
            PriorityMedium = 1u << 0,       ///< Medium priority
            PriorityHigh = 2u << 0,         ///< High priority
            PriorityVeryHigh = 3u << 0,     ///< Very high priority

            MSize8Bits = 0u << 2,           ///< Memory size 8 bits
            MSize16Bits = 1u << 2,          ///< Memory size 16 bits
            MSize32Bits = 2u << 2,          ///< Memory size 32 bits

            PSize8Bits = 0u << 4,           ///< Peripheral size 8 bits
            PSize16Bits = 1u << 4,          ///< Peripheral size 16 bits
            PSize32Bits = 2u << 4,          ///< Peripheral size 32 bits

            MemIncrement = 1u << 6,         ///< Enable memory increment
            PeriphIncrement = 1u << 7,      ///< Enable peripheral increment
            Circular = 1u << 8,             ///< Circular transfer (self-linked LLI)

            Periph2Mem = 0u << 9,           ///< Peripheral to memory transfer
            Mem2Periph = 1u << 9,           ///< Memory to peripheral transfer
            Mem2Mem = 2u << 9,              ///< Memory to memory transfer

            TransferErrorInterrupt = 1u << 11,      ///< Transfer error interrupt
            HalfTransferInterrupt = 1u << 12,       ///< Half transfer interrupt
            TransferCompleteInterrupt = 1u << 13,   ///< Transfer complete interrupt
        };

    protected:
        static constexpr uint32_t PriorityMask = 0x3u << 0;
        static constexpr uint32_t MemorySizeMask = 0x3u << 2;
        static constexpr uint32_t PeriphSizeMask = 0x3u << 4;
        static constexpr uint32_t DirectionMask = 0x3u << 9;

        static constexpr uint32_t MemorySizeShift = 2;
        static constexpr uint32_t PeriphSizeShift = 4;
        static constexpr uint32_t DirectionShift = 9;
    };

    /**
     * @brief DMA channel data
     */
    struct DmaChannelData
    {
        using TransferCallback = std::add_pointer_t<void(void* data, unsigned size, bool success)>;

        DmaChannelData()
            : transferCallback(nullptr),
            data(nullptr),
            size(0)
        {}

        TransferCallback transferCallback; ///< Transfer complete/error callback pointer

        void* data; ///< Data buffer
        uint16_t size; ///< Data buffer size

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
     * @brief Linked-list item used to express a circular transfer
     *
     * @details
     * GPDMA reloads the registers selected by CxLLR.U* from memory in a fixed
     * order (CTR1, CTR2, CBR1, CSAR, CDAR, CTR3, CBR2, CLLR). A repeating
     * transfer only needs the byte count, the moving (memory) address and the
     * pointer back to the item itself, so the item is exactly three words.
     */
    struct alignas(16) DmaLinkedListItem
    {
        uint32_t BlockSize;     ///< Reloaded into CxBR1
        uint32_t MemoryAddress; ///< Reloaded into CxSAR or CxDAR
        uint32_t NextItem;      ///< Reloaded into CxLLR (points back here)
    };

    /**
     * @brief Implements a GPDMA channel
     *
     * @tparam _Module Parent DMA module
     * @tparam _ChannelRegs Pointer to associated DMA_Channel_TypeDef structure
     * @tparam _Channel Channel number (0-based, as GPDMA numbers them)
     * @tparam _IRQNumber DMA channel IRQ number
     */
    template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQNumber>
    class DmaChannel : public DmaBase
    {
        static_assert(_Channel < _Module::Channels);

        static DmaChannelData Data;
        static DmaLinkedListItem LinkedListItem;
        /// Request line selected through DmaMux::Channel::SelectRequestInput, kept across transfers
        static uint8_t RequestInput;

        static constexpr uint32_t AllFlags = DMA_CFCR_TCF | DMA_CFCR_HTF | DMA_CFCR_DTEF
            | DMA_CFCR_ULEF | DMA_CFCR_USEF | DMA_CFCR_SUSPF | DMA_CFCR_TOF;
        static constexpr uint32_t ErrorFlags = DMA_CSR_DTEF | DMA_CSR_ULEF | DMA_CSR_USEF;

    public:
        using Module = _Module;
        using DmaBase::Mode;
        static constexpr unsigned Channel = _Channel;

        /**
         * @brief Select the hardware request driving this channel
         *
         * @details
         * The value survives Transfer() calls, mirroring how DMAMUX behaves on
         * the other families.
         *
         * @param [in] request GPDMA request line number (RM0492 table 87)
         *
         * @par Returns
         *  Nothing
         */
        static void SelectRequestInput(uint8_t request)
        {
            RequestInput = request;
        }

        /**
         * @brief Returns currently selected hardware request
         *
         * @returns Request line number
         */
        static uint8_t GetRequestInput()
        {
            return RequestInput;
        }

        /**
         * @brief Initialize DMA channel and start transfer
         *
         * @details
         * @p bufferSize is a number of items (as on the other families); GPDMA
         * counts bytes, so it is scaled by the memory-side item width here.
         *
         * @param [in] mode Channel mode (flags may be OR-ed together)
         * @param [in] buffer Memory buffer
         * @param [in] periph Peripheral address (or second memory buffer for Mem2Mem)
         * @param [in] bufferSize Item count
         *
         * @par Returns
         *  Nothing
         */
        static void Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize)
        {
            _Module::Enable();

            Disable();
            ClearFlags();

            auto flags = static_cast<uint32_t>(mode);
            uint32_t direction = (flags & DirectionMask) >> DirectionShift;
            uint32_t memoryWidth = (flags & MemorySizeMask) >> MemorySizeShift;
            uint32_t periphWidth = (flags & PeriphSizeMask) >> PeriphSizeShift;
            bool memoryIncrement = (flags & MemIncrement) != 0;
            bool periphIncrement = (flags & PeriphIncrement) != 0;

            uint32_t blockSize = bufferSize << memoryWidth;

            uint32_t sourceWidth, destinationWidth;
            bool sourceIncrement, destinationIncrement;
            uint32_t sourceAddress, destinationAddress;
            uint32_t transfer2 = 0;

            if (direction == (Mem2Periph >> DirectionShift))
            {
                sourceAddress = reinterpret_cast<uint32_t>(buffer);
                destinationAddress = reinterpret_cast<uint32_t>(periph);
                sourceWidth = memoryWidth;
                destinationWidth = periphWidth;
                sourceIncrement = memoryIncrement;
                destinationIncrement = periphIncrement;
                // The request comes from the destination peripheral
                transfer2 = DMA_CTR2_DREQ | RequestInput;
            }
            else if (direction == (Mem2Mem >> DirectionShift))
            {
                sourceAddress = reinterpret_cast<uint32_t>(periph);
                destinationAddress = reinterpret_cast<uint32_t>(buffer);
                sourceWidth = periphWidth;
                destinationWidth = memoryWidth;
                sourceIncrement = periphIncrement;
                destinationIncrement = memoryIncrement;
                transfer2 = DMA_CTR2_SWREQ;
            }
            else
            {
                sourceAddress = reinterpret_cast<uint32_t>(periph);
                destinationAddress = reinterpret_cast<uint32_t>(buffer);
                sourceWidth = periphWidth;
                destinationWidth = memoryWidth;
                sourceIncrement = periphIncrement;
                destinationIncrement = memoryIncrement;
                blockSize = bufferSize << periphWidth;
                transfer2 = RequestInput;
            }

            Data.data = const_cast<void*>(buffer);
            Data.size = static_cast<uint16_t>(bufferSize);

            if (Data.transferCallback)
                flags |= (TransferCompleteInterrupt | TransferErrorInterrupt);

            _ChannelRegs()->CTR1 = (sourceWidth << DMA_CTR1_SDW_LOG2_Pos)
                | (sourceIncrement ? DMA_CTR1_SINC : 0u)
                | (destinationWidth << DMA_CTR1_DDW_LOG2_Pos)
                | (destinationIncrement ? DMA_CTR1_DINC : 0u);
            _ChannelRegs()->CTR2 = transfer2;
            _ChannelRegs()->CBR1 = blockSize;
            _ChannelRegs()->CSAR = sourceAddress;
            _ChannelRegs()->CDAR = destinationAddress;

            if ((flags & Circular) != 0)
            {
                // Reload the byte count, the moving address and the link itself,
                // so the item re-arms the very same transfer forever.
                bool memoryIsDestination = direction != (Mem2Periph >> DirectionShift);

                LinkedListItem.BlockSize = blockSize;
                LinkedListItem.MemoryAddress = memoryIsDestination ? destinationAddress : sourceAddress;

                auto itemAddress = reinterpret_cast<uint32_t>(&LinkedListItem);
                uint32_t link = DMA_CLLR_UB1 | DMA_CLLR_ULL | (itemAddress & DMA_CLLR_LA_Msk)
                    | (memoryIsDestination ? DMA_CLLR_UDA : DMA_CLLR_USA);

                LinkedListItem.NextItem = link;

                _ChannelRegs()->CLBAR = itemAddress & 0xFFFF0000u;
                _ChannelRegs()->CLLR = link;
            }
            else
            {
                _ChannelRegs()->CLLR = 0;
            }

            NVIC_EnableIRQ(_IRQNumber);

            _ChannelRegs()->CCR = ((flags & PriorityMask) << DMA_CCR_PRIO_Pos)
                | ((flags & TransferCompleteInterrupt) != 0 ? DMA_CCR_TCIE : 0u)
                | ((flags & HalfTransferInterrupt) != 0 ? DMA_CCR_HTIE : 0u)
                | ((flags & TransferErrorInterrupt) != 0 ? (DMA_CCR_DTEIE | DMA_CCR_ULEIE | DMA_CCR_USEIE) : 0u)
                | DMA_CCR_EN;
        }

        /**
         * @brief Set transfer callback function
         *
         * @param [in] callback Pointer to callback function
         *
         * @par Returns
         *  Nothing
         */
        static void SetTransferCallback(DmaChannelData::TransferCallback callback)
        {
            Data.transferCallback = callback;
        }

        /**
         * @brief Check that DMA is ready to transfer data
         *
         * @retval true DMA ready
         * @retval false DMA not ready
         */
        static bool Ready()
        {
            return RemainingTransfers() == 0 || !Enabled() || TransferComplete();
        }

        /**
         * @brief Check DMA channel state
         *
         * @retval true DMA channel enabled
         * @retval false DMA channel disabled
         */
        static bool Enabled()
        {
            return (_ChannelRegs()->CCR & DMA_CCR_EN) != 0;
        }

        /**
         * @brief Enable DMA channel
         *
         * @par Returns
         *  Nothing
         */
        static void Enable()
        {
            _ChannelRegs()->CCR |= DMA_CCR_EN;
        }

        /**
         * @brief Disable DMA channel
         *
         * @details
         * GPDMA ignores a 0 written to EN: an active channel must be suspended
         * and then reset.
         *
         * @par Returns
         *  Nothing
         */
        static void Disable()
        {
            if (Enabled())
            {
                _ChannelRegs()->CCR |= DMA_CCR_SUSP;
                while ((_ChannelRegs()->CSR & DMA_CSR_SUSPF) == 0)
                    ;
            }

            _ChannelRegs()->CCR = DMA_CCR_RESET;
            while ((_ChannelRegs()->CSR & DMA_CSR_IDLEF) == 0)
                ;
        }

        /**
         * @brief Returns remaining bytes to transfer
         *
         * @details
         * Unlike the CNDTR-based families this counts *bytes*, since that is
         * what GPDMA_CxBR1.BNDT holds.
         *
         * @returns Bytes remaining
         */
        static uint32_t RemainingTransfers()
        {
            return _ChannelRegs()->CBR1 & DMA_CBR1_BNDT_Msk;
        }

        /**
         * @brief Returns peripheral address
         *
         * @returns Peripheral address
         */
        static void* PeriphAddress()
        {
            return reinterpret_cast<void*>((_ChannelRegs()->CTR2 & DMA_CTR2_DREQ) != 0
                ? _ChannelRegs()->CDAR
                : _ChannelRegs()->CSAR);
        }

        /**
         * @brief Returns memory buffer address
         *
         * @returns Memory buffer address
         */
        static void* MemAddress()
        {
            return reinterpret_cast<void*>((_ChannelRegs()->CTR2 & DMA_CTR2_DREQ) != 0
                ? _ChannelRegs()->CSAR
                : _ChannelRegs()->CDAR);
        }

        /**
         * @brief Returns transfer error state
         *
         * @retval true A transfer, link or user setting error was flagged
         * @retval false No error
         */
        static bool TransferError()
        {
            return (_ChannelRegs()->CSR & ErrorFlags) != 0;
        }

        /**
         * @brief Returns half transfer state
         *
         * @retval true Half of the data was transferred
         * @retval false Not yet
         */
        static bool HalfTransfer()
        {
            return (_ChannelRegs()->CSR & DMA_CSR_HTF) != 0;
        }

        /**
         * @brief Returns transfer complete state
         *
         * @retval true Transfer complete
         * @retval false Transfer not complete
         */
        static bool TransferComplete()
        {
            return (_ChannelRegs()->CSR & DMA_CSR_TCF) != 0;
        }

        /**
         * @brief Clear all channel flags
         *
         * @par Returns
         *  Nothing
         */
        static void ClearFlags()
        {
            _ChannelRegs()->CFCR = AllFlags;
        }

        /**
         * @brief Clear transfer error flags
         *
         * @par Returns
         *  Nothing
         */
        static void ClearTransferError()
        {
            _ChannelRegs()->CFCR = DMA_CFCR_DTEF | DMA_CFCR_ULEF | DMA_CFCR_USEF;
        }

        /**
         * @brief Clear half transfer complete flag
         *
         * @par Returns
         *  Nothing
         */
        static void ClearHalfTransfer()
        {
            _ChannelRegs()->CFCR = DMA_CFCR_HTF;
        }

        /**
         * @brief Clear transfer complete flag
         *
         * @par Returns
         *  Nothing
         */
        static void ClearTransferComplete()
        {
            _ChannelRegs()->CFCR = DMA_CFCR_TCF;
        }

        /**
         * @brief DMA channel IRQ handler
         *
         * @par Returns
         *  Nothing
         */
        static void IrqHandler()
        {
            bool circular = _ChannelRegs()->CLLR != 0;

            if (TransferComplete())
            {
                ClearFlags();

                if (!circular)
                    Disable();

                Data.NotifyTransferComplete();
            }
            if (TransferError())
            {
                ClearFlags();
                Disable();

                Data.NotifyError();
            }
        }
    };

    /**
     * @brief Implements a GPDMA controller
     *
     * @tparam _DmaRegs Pointer to associated DMA_TypeDef structure
     * @tparam _Clock Source clock class
     * @tparam _Channels DMA channels count
     */
    template<typename _DmaRegs, typename _Clock, unsigned _Channels>
    class DmaModule : public DmaBase
    {
    public:
        static const int Channels = _Channels;

        /**
         * @brief Enable DMA module clocking
         *
         * @par Returns
         *  Nothing
         */
        static void Enable()
        {
            _Clock::Enable();
        }

        /**
         * @brief Disable DMA module clocking
         *
         * @par Returns
         *  Nothing
         */
        static void Disable()
        {
            _Clock::Disable();
        }
    };

    template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQnumber>
    DmaChannelData DmaChannel<_Module, _ChannelRegs, _Channel, _IRQnumber>::Data;

    template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQnumber>
    DmaLinkedListItem DmaChannel<_Module, _ChannelRegs, _Channel, _IRQnumber>::LinkedListItem;

    template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQnumber>
    uint8_t DmaChannel<_Module, _ChannelRegs, _Channel, _IRQnumber>::RequestInput;

    namespace Private
    {
        IO_STRUCT_WRAPPER(GPDMA1, GpDma1, DMA_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel0, GpDma1Channel0Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel1, GpDma1Channel1Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel2, GpDma1Channel2Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel3, GpDma1Channel3Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel4, GpDma1Channel4Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel5, GpDma1Channel5Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel6, GpDma1Channel6Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA1_Channel7, GpDma1Channel7Regs, DMA_Channel_TypeDef);

        IO_STRUCT_WRAPPER(GPDMA2, GpDma2, DMA_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel0, GpDma2Channel0Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel1, GpDma2Channel1Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel2, GpDma2Channel2Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel3, GpDma2Channel3Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel4, GpDma2Channel4Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel5, GpDma2Channel5Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel6, GpDma2Channel6Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(GPDMA2_Channel7, GpDma2Channel7Regs, DMA_Channel_TypeDef);
    }

    using Dma1 = DmaModule<Private::GpDma1, Clock::GpDma1Clock, 8>;
    using Dma1Channel0 = DmaChannel<Dma1, Private::GpDma1Channel0Regs, 0, GPDMA1_Channel0_IRQn>;
    using Dma1Channel1 = DmaChannel<Dma1, Private::GpDma1Channel1Regs, 1, GPDMA1_Channel1_IRQn>;
    using Dma1Channel2 = DmaChannel<Dma1, Private::GpDma1Channel2Regs, 2, GPDMA1_Channel2_IRQn>;
    using Dma1Channel3 = DmaChannel<Dma1, Private::GpDma1Channel3Regs, 3, GPDMA1_Channel3_IRQn>;
    using Dma1Channel4 = DmaChannel<Dma1, Private::GpDma1Channel4Regs, 4, GPDMA1_Channel4_IRQn>;
    using Dma1Channel5 = DmaChannel<Dma1, Private::GpDma1Channel5Regs, 5, GPDMA1_Channel5_IRQn>;
    using Dma1Channel6 = DmaChannel<Dma1, Private::GpDma1Channel6Regs, 6, GPDMA1_Channel6_IRQn>;
    using Dma1Channel7 = DmaChannel<Dma1, Private::GpDma1Channel7Regs, 7, GPDMA1_Channel7_IRQn>;

    using Dma2 = DmaModule<Private::GpDma2, Clock::GpDma2Clock, 8>;
    using Dma2Channel0 = DmaChannel<Dma2, Private::GpDma2Channel0Regs, 0, GPDMA2_Channel0_IRQn>;
    using Dma2Channel1 = DmaChannel<Dma2, Private::GpDma2Channel1Regs, 1, GPDMA2_Channel1_IRQn>;
    using Dma2Channel2 = DmaChannel<Dma2, Private::GpDma2Channel2Regs, 2, GPDMA2_Channel2_IRQn>;
    using Dma2Channel3 = DmaChannel<Dma2, Private::GpDma2Channel3Regs, 3, GPDMA2_Channel3_IRQn>;
    using Dma2Channel4 = DmaChannel<Dma2, Private::GpDma2Channel4Regs, 4, GPDMA2_Channel4_IRQn>;
    using Dma2Channel5 = DmaChannel<Dma2, Private::GpDma2Channel5Regs, 5, GPDMA2_Channel5_IRQn>;
    using Dma2Channel6 = DmaChannel<Dma2, Private::GpDma2Channel6Regs, 6, GPDMA2_Channel6_IRQn>;
    using Dma2Channel7 = DmaChannel<Dma2, Private::GpDma2Channel7Regs, 7, GPDMA2_Channel7_IRQn>;

    // GPDMA1 and GPDMA2 are identical: expose the "Gp" spelling as well
    using GpDma1 = Dma1;
    using GpDma2 = Dma2;
} // namespace Zhele

#endif //! ZHELE_PLATFORM_STM32_H5_DMA_H
