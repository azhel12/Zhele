/**
 * @file
 * Implement code for shared resource of endpoints
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_ENDPOINTS_MANAGER_H
#define ZHELE_USB_ENDPOINTS_MANAGER_H

#include "../template_utils/static_array.h"
#include "../template_utils/type_list.h"

#include "endpoint.h"

#include <type_traits>
#include <stdint.h>
#include <string.h>

namespace Zhele::Usb
{
    using namespace Zhele::TemplateUtils;
    
    /**
     * @brief Endpoint transfer complete callback.
     */
    using EpRequestHandler = std::add_pointer_t<void()>;

    /**
     * @brief Endpoint transfer complete callback.
     */
    using EpRxFifoNotEmptyHandler = std::add_pointer_t<void(uint16_t size)>;

    /**
     * @brief Predicate for search Tx endpoint by number
     * 
     * @tparam Number Endpoint number
     */
    template<uint8_t Number>
    class IsTxEndpointWithNumber
    {
    public:
        template<typename Endpoint>
        class type
        {
        public:
            static const bool value = (Endpoint::Number == Number && Endpoint::Direction == EndpointDirection::Out);
        };
    };

    /**
     * @brief Predicate for search Rx endpoint by number
     * 
     * @tparam Number Endpoint number
     */
    template<uint8_t Number>
    class IsRxEndpointWithNumber
    {
    public:
        template<typename Endpoint>
        class type
        {
        public:
            static const bool value = (Endpoint::Number == Number && Endpoint::Direction == EndpointDirection::In);
        };
    };

    /**
     * @brief Predicate for search Tx or bidirectional endpoint by number
     * 
     * @tparam Number Endpoint number
     */
    template<uint8_t Number>
    class IsTxOrBidirectionalEndpointWithNumber
    {
    public:
        template<typename Endpoint>
        class type
        {
        public:
            static const bool value = Endpoint::Number == Number && (Endpoint::Direction == EndpointDirection::In || Endpoint::Direction == EndpointDirection::Bidirectional);
        };
    };

    /**
     * @brief Predicate for search Rx or bidirectional endpoint by number
     * 
     * @tparam Number Endpoint number
     */
    template<uint8_t Number>
    class IsRxOrBidirectionalEndpointWithNumber
    {
    public:
        template<typename Endpoint>
        class type
        {
        public:
            static const bool value = Endpoint::Number == Number && (Endpoint::Direction == EndpointDirection::Out || Endpoint::Direction == EndpointDirection::Bidirectional);
        };
    };

    /**
     * @brief Predicat for search Rx endpoint by number
     * 
     * @tparam Number Endpoint number
     */
    template<uint8_t Number>
    class IsBidirectionalEndpointWithNumber
    {
    public:
        template<typename Endpoint>
        class type
        {
        public:
            static const bool value = (Endpoint::Number == Number && Endpoint::Direction == EndpointDirection::Bidirectional);
        };
    };

    /**
     * @brief Predicate for search control or double-buffered bulk endpoints.
     * 
     * @tparam Endpoint Endpoint
     */
    template<typename Endpoint>
    class IsBidirectionalOrBulkDoubleBufferedEndpoint
    {
    public:
        static const bool value = (Endpoint::Type == EndpointType::BulkDoubleBuffered
                                || Endpoint::Direction == EndpointDirection::Bidirectional);
    };

    /**
     * @brief Predicate for search Rx (no control) endpoints.
     * 
     * @tparam Endpoint Endpoint
     */
    template<typename Endpoint>
    class IsOutEndpoint
    {
    public:
        static const bool value = (Endpoint::Direction == EndpointDirection::Out
                                && Endpoint::Type != EndpointType::Control
                                && Endpoint::Type != EndpointType::ControlStatusOut);
    };

    /**
     * @brief Predicate for search Tx double-buffered
     * 
     * @tparam Endpoint Endpoint
     */
    template<typename Endpoint>
    class IsBulkDoubleBufferedTxEndpoint
    {
    public:
        static const bool value = (Endpoint::Type == EndpointType::BulkDoubleBuffered && Endpoint::Direction == EndpointDirection::Out);
    };

    /**
     * @brief Predicate for search Rx (no control) endpoints.
     * 
     * @tparam Endpoint Endpoint
     */
    template<typename Endpoint>
    class IsOutOrBidirectionalEndpoint
    {
    public:
        static const bool value = Endpoint::Direction == EndpointDirection::Out || Endpoint::Direction == EndpointDirection::Bidirectional;
    };

    /**
     * @brief Predicate for search Rx (no control) endpoints.
     * 
     * @tparam Endpoint Endpoint
     */
    template<typename Endpoint>
    class IsInOrBidirectionalEndpoint
    {
    public:
        static const bool value = Endpoint::Direction == EndpointDirection::In || Endpoint::Direction == EndpointDirection::Bidirectional;
    };


    /**
     * @brief Null endpoint (useful for search, sort)
     */
    class NullEndpoint
    {
    public:
        static const uint16_t Number = -1;
        static const EndpointDirection Direction = EndpointDirection::In;
        static const EndpointType Type = EndpointType::Control;
    };
#if defined (USB)
    /**
     * @brief Comparator for endpoints sort (compare Number/Direction)
     * 
     * @tparam T First endpoint
     * @tparam U Second endpoint
     */
    template <typename T, typename U>
    struct NumberAndDirectionComparator : std::conditional_t<
        (U::Number < T::Number || (U::Number == T::Number && static_cast<uint8_t>(U::Direction) > static_cast<uint8_t>(T::Direction))),
        std::true_type,
        std::false_type>
    {};
#elif defined (USB_OTG_FS)
    /**
     * @brief Comparator for endpoints sort (compare Number numbers only, because its different registers for IN/OUT endpoints) for OTG
     * 
     * @tparam T First endpoint
     * @tparam U Second endpoint
     */
    template <typename T, typename U>
    struct NumberAndDirectionComparator : std::conditional_t<
                U::Number < T::Number,
        std::true_type,
        std::false_type>
    {};
#endif

    /**
     * @brief Sorts endpoints by number and direction.
     * 
     * @tparam Endpoints Endpoints list
     */
    template<typename Endpoints>
    using EndpointsSortedByNumberAndDirection = typename TypeListSort<NumberAndDirectionComparator, Endpoints>::type;

    /**
     * @brief Unique endpoints sorted by numbers/direction.
     * 
     * @tparam Endpoints Endpoints list
     */
    template<typename Endpoints>
    using SortedUniqueEndpoints = EndpointsSortedByNumberAndDirection<typename Unique<Endpoints>::type>;

    /**
     * @brief Calculates endpoint`s buffers offsets.
     * 
     * @tparam Index Endpoint index in list
     * @tparam Endpoints Endpoints list
     */
    template<int Index, typename Endpoints>
    class OffsetOfBuffer;
    template<typename... Endpoints>
    class OffsetOfBuffer<0, TypeList<Endpoints...>>
    {
    public:
        static const unsigned value = 0;
    };
    template<int Index, typename... Endpoints>
    class OffsetOfBuffer<Index, TypeList<Endpoints...>>
    {
        using Endpoint = GetType<Index, TypeList<Endpoints...>>::type;
        using PreviousEndpoint = GetType<Index - 1, TypeList<Endpoints...>>::type;
    public:
        static const uint16_t value = OffsetOfBuffer<Index - 1, TypeList<Endpoints...>>::value +
            ((PreviousEndpoint::Type == EndpointType::BulkDoubleBuffered
            || PreviousEndpoint::Direction == EndpointDirection::Bidirectional)
                ? PreviousEndpoint::MaxPacketSize * 2
                : PreviousEndpoint::MaxPacketSize);
    };

#if defined (USB)
    /**
     * @brief Calculates endpoint`s BDT cells offsets.
     * 
     * @tparam Index Endpoint index in list
     * @tparam Endpoints Endpoints list
     */
    template<int Index, typename Endpoints>
    class OffsetOfPacketDescriptor;
    template<typename... Endpoints>
    class OffsetOfPacketDescriptor<-1, TypeList<Endpoints...>>
    {
    public:
        const unsigned value = 0;
    };
    template<int Number, typename... Endpoints>
    class OffsetOfPacketDescriptor<Number, TypeList<Endpoints...>>
    {
        using Endpoint = GetType<Number, TypeList<Endpoints...>>::type;
    public:
        const unsigned value = Endpoint::Type == EndpointType::BulkDoubleBuffered
            ? 4 + OffsetOfBuffer<Number - 1, TypeList<Endpoints...>>::value
            : 2 + OffsetOfBuffer<Number - 1, TypeList<Endpoints...>>::value;
    };

    /// Wrappers for EPnR registers.
    /// C++ doesn't allows reinterpret_cast in compile-time.
    IO_REG_WRAPPER(USB->EP0R, Ep0Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP1R, Ep1Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP2R, Ep2Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP3R, Ep3Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP4R, Ep4Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP5R, Ep5Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP6R, Ep6Reg, uint16_t);
    IO_REG_WRAPPER(USB->EP7R, Ep7Reg, uint16_t);
    using EndpointRegs = Zhele::TemplateUtils::TypeList<Ep0Reg, Ep1Reg, Ep2Reg, Ep3Reg, Ep4Reg, Ep5Reg, Ep6Reg, Ep7Reg>;

    /**
     * @brief Select endpoint register by number
     */
    template<uint8_t _EndpointNumber>
    using EndpointReg = Zhele::TemplateUtils::GetType<_EndpointNumber, EndpointRegs>::type;

    /**
     * @brief Calculates endpoint`s registers.
     * 
     * @details Endpoints can have any address 0..15, but MCU contains only 8 registers.
     * So, you can declare 3 endpoints with numbers 0, 6, 10 (for example) and this class
     * assigns EP0R to Ep0, EP1R to Ep6 and EP2R to Ep10
     * 
     * @tparam Endpoint Endpoint
     * @tparam Endpoints All endpoints list
     */
    template<typename Endpoint, typename Endpoints>
    class EndpointEPRn;
    template<typename... Endpoints>
    class EndpointEPRn<NullEndpoint, TypeList<Endpoints...>>
    {
    public:
        static const uint8_t RegisterNumber = -1;
    };
    template<typename Endpoint, typename... Endpoints>
    class EndpointEPRn<Endpoint, TypeList<Endpoints...>>
    {
        static const int index = TypeIndex<Endpoint, TypeList<Endpoints...>>::value;
        using PreviousEndpoint = typename Select<(index > 0), typename GetType<(index - 1), TypeList<Endpoints...>>::type, NullEndpoint>::value;

        static const bool IsEndpointNumberEqualToPreviousEndpointNumber =
            index > 1
                ? Endpoint::Number == PreviousEndpoint::Number
                : false;

        // Two endpoints can share one EPnR (if they are unidirectional, not control and double-buffered Bulk)
        static const bool IsEndpointIncompatibleWithPrevious = IsEndpointNumberEqualToPreviousEndpointNumber
            && (Endpoint::Type == EndpointType::Control
                || Endpoint::Type == EndpointType::BulkDoubleBuffered
                || Endpoint::Direction == EndpointDirection::Bidirectional
                || PreviousEndpoint::Type == EndpointType::Control
                || PreviousEndpoint::Type == EndpointType::BulkDoubleBuffered
                || PreviousEndpoint::Direction == EndpointDirection::Bidirectional);

        static_assert(!IsEndpointIncompatibleWithPrevious, "Incompatible endpoints with same number");

    public:
        static const uint8_t RegisterNumber =
            index == 0
                ? 0
                : (IsEndpointNumberEqualToPreviousEndpointNumber
                    ? EndpointEPRn<PreviousEndpoint, TypeList<Endpoints...>>::RegisterNumber
                    : EndpointEPRn<PreviousEndpoint, TypeList<Endpoints...>>::RegisterNumber + 1);

        using type = EndpointReg<RegisterNumber>;
    };

    /**
     * @brief Implements endpoint`s buffers management.
     * 
     * @tparam AllEndpoints Sorted all endpoints list
     * @tparam BidirectionalAndBulkDoubleBufferedEndpoints Sorted bidirectional and double-buffered bulk endpoints
     * @tparam RxEndpoints Sorted RX endpoints (include double-buffered Bulk)
     */
    template<typename AllEndpoints, typename BidirectionalAndBulkDoubleBufferedEndpoints, typename RxEndpoints>
    class EndpointsManagerBase;
    template<typename... AllEndpoints, typename... BidirectionalAndBulkDoubleBufferedEndpoints, typename... RxEndpoints>
    class EndpointsManagerBase<TypeList<AllEndpoints...>, TypeList<BidirectionalAndBulkDoubleBufferedEndpoints...>, TypeList<RxEndpoints...>>
    {
        /// USB PMA base address
        static const uint32_t PmaBufferBase = USB_PMAADDR;
    public:
        using AllEndpointsList = TypeList<AllEndpoints...>;

        /// Buffer descriptor table size (all realy used endpoints * 8)
        static const auto BdtSize = 8 * (EndpointEPRn<GetType_t<sizeof...(AllEndpoints) - 1, AllEndpointsList>, AllEndpointsList>::RegisterNumber + 1);

        /// Buffer offset in PMA for endpoint
        template<typename Endpoint>
        static const uint32_t BufferOffset = BdtSize + OffsetOfBuffer<TypeIndex<Endpoint, AllEndpointsList>::value, AllEndpointsList>::value;

        /// Buffer descriptor offset in BDT for endpoint
        template<typename Endpoint>
        static const uint32_t BdtCellOffset =
            EndpointEPRn<Endpoint, AllEndpointsList>::RegisterNumber * 8
                                + (Endpoint::Type == EndpointType::BulkDoubleBuffered
                                || Endpoint::Direction == EndpointDirection::In
                                || Endpoint::Direction == EndpointDirection::Bidirectional
                                    ? 0
                                    : 4);
        static const uint32_t BdtBase = PmaBufferBase;
    public:
        /**
         * @brief Extends endpoint (init addresses)
         * 
         * @tparam Endpoint Endpoint to extend
         */
        template<typename Endpoint>
        using ExtendEndpoint = 
            typename Select<Endpoint::Direction == EndpointDirection::Bidirectional,
            BidirectionalEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // TxBuffer
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // TxCount
                PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + Endpoint::MaxPacketSize), // RxBuffer
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //RxCount
            typename Select<Endpoint::Type == EndpointType::BulkDoubleBuffered,
            BulkDoubleBufferedEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer0
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // Buffer0Count
                PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + Endpoint::MaxPacketSize), // Buffer1
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //Buffer1Count
            typename Select<Endpoint::Direction == EndpointDirection::In,
            InEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)>, // BufferCount
            typename Select<Endpoint::Direction == EndpointDirection::Out,
            OutEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer
                PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)>, // BufferCount
            void>::value>::value>::value>::value;

        /**
         * @brief Inits USB PMA
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            (InitTxFieldsInDescriptor<AllEndpoints>(), ...);
            (InitRxAddressFieldInDescriptor<BidirectionalAndBulkDoubleBufferedEndpoints>(), ...);
            (InitRxCountFieldInDescriptor<RxEndpoints>(), ...);
            (InitSecondRxCountFieldInDescriptor<BidirectionalAndBulkDoubleBufferedEndpoints>(), ...);
        }

    private:
        template<typename Endpoint>
        static void InitTxFieldsInDescriptor()
        {
            *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * BdtCellOffset<Endpoint>) = BufferOffset<Endpoint>;
            *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)) = 0u;
        }
        template<typename Endpoint>
        static void InitRxAddressFieldInDescriptor()
        {
            *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 4)) = BufferOffset<Endpoint> + Endpoint::MaxPacketSize;
        }
        template<typename Endpoint>
        static void InitRxCountFieldInDescriptor()
        {
            *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)) = CalculateRxCountValue<Endpoint>();
        }
        template<typename Endpoint>
        static void InitSecondRxCountFieldInDescriptor()
        {
            *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)) = CalculateRxCountValue<Endpoint>();
        }
        template<typename Endpoint>
        static constexpr uint16_t CalculateRxCountValue()
        {
            return Endpoint::MaxPacketSize <= 62
                ? (Endpoint::MaxPacketSize / 2) << 10
                : 0x8000 | ((Endpoint::MaxPacketSize / 32 - 1) << 10);
        }
    };

    /**
     * @brief Endpoints manager.
     * 
     * @tparam Endpoints ALl endpoints list
     */
    template<typename Endpoints>
    using EndpointsManager = EndpointsManagerBase
    <
        SortedUniqueEndpoints<Endpoints>,
        Sample_t<IsBidirectionalOrBulkDoubleBufferedEndpoint, SortedUniqueEndpoints<Endpoints>>,
        Sample_t<IsOutEndpoint, SortedUniqueEndpoints<Endpoints>>
    >;

#elif defined (USB_OTG_FS)
    #define USB_INEP(i)  (USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (i) * USB_OTG_EP_REG_SIZE)
    #define USB_OUTEP(i) (USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (i) * USB_OTG_EP_REG_SIZE)

    IO_STRUCT_WRAPPER(USB_INEP(0), InEp0Reg, USB_OTG_INEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_INEP(1), InEp1Reg, USB_OTG_INEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_INEP(2), InEp2Reg, USB_OTG_INEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_INEP(3), InEp3Reg, USB_OTG_INEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_OUTEP(0), OutEp0Reg, USB_OTG_OUTEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_OUTEP(1), OutEp1Reg, USB_OTG_OUTEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_OUTEP(2), OutEp2Reg, USB_OTG_OUTEndpointTypeDef);
    IO_STRUCT_WRAPPER(USB_OUTEP(3), OutEp3Reg, USB_OTG_OUTEndpointTypeDef);
    using EndpointInRegs = Zhele::TemplateUtils::TypeList<InEp0Reg, InEp1Reg, InEp2Reg, InEp3Reg>;
    using EndpointOutRegs = Zhele::TemplateUtils::TypeList<OutEp0Reg, OutEp1Reg, OutEp2Reg, OutEp3Reg>;
    
    /**
     * @brief Select endpoint register by number
     */
    template<uint8_t _EndpointNumber>
    using EndpointInReg = Zhele::TemplateUtils::GetType<_EndpointNumber, EndpointInRegs>::type;
    template<uint8_t _EndpointNumber>
    using EndpointOutReg = Zhele::TemplateUtils::GetType<_EndpointNumber, EndpointOutRegs>::type;

    template<typename AllEndpoints, typename InEndpoints, typename OutEndpoints>
    class EndpointsManagerBase;
    template<typename... AllEndpoints, typename... InEndpoints, typename... OutEndpoints>
    class EndpointsManagerBase<TypeList<AllEndpoints...>, TypeList<InEndpoints...>, TypeList<OutEndpoints...>>
    {
        IO_STRUCT_WRAPPER(USB_OTG_FS, OtgFsGlobal, USB_OTG_GlobalTypeDef);

        /// Buffer offset for endpoint
        template<typename Endpoint>
        static const uint32_t BufferOffset = OffsetOfBuffer<TypeIndex<Endpoint, TypeList<OutEndpoints...>>::value, TypeList<OutEndpoints...>>::value;


        /**
         * @brief Comparator for endpoints sort by MaxPacketSize
         * 
         * @details RX fifo size recommended as 2x maximum of all OUT endpoint`s max packet size (+ 11 dwords. Minimum = 16)
         * So, we can sort OUT endpoint by max packet size, get maximum and calculate RX fifo size.
         * 
         * @tparam T First endpoint
         * @tparam U Second endpoint
         */
        template <typename T, typename U>
        struct EndpointMaxPacketSizeComparator : std::conditional_t<(U::MaxPacketSize > T::MaxPacketSize), std::true_type, std::false_type> {};
        /// OUT (bidirectional) endpoints sorted by MaxPacketSize descending.
        using OutEndpointsSortedByMaxPacketSizeDescending = typename TypeListSort<EndpointMaxPacketSizeComparator, TypeList<OutEndpoints...>>::type;
        /// Maximum of MaxPacketSize
        static const uint16_t MaximumOfOutEnpointsMaxPacketSize = GetType<0, OutEndpointsSortedByMaxPacketSizeDescending>::type::MaxPacketSize;

        /// RX fifo size calculation
        static const uint16_t RxFifoSize = (11 + 2 * ((3 + MaximumOfOutEnpointsMaxPacketSize) / 4)) > 16 // 11 = 10 (SETUP) + 1 (global out NAK)
            ? 11 + 2 * ((3 + MaximumOfOutEnpointsMaxPacketSize) / 4)
            : 16;

        static const uint32_t FifoBaseAddress = USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE;
        static const uint32_t EpFifoSize = USB_OTG_FIFO_SIZE;

        using InEndpointsList = TypeList<InEndpoints...>;
        using OutEndpointsList = TypeList<OutEndpoints...>;
        using InEndpointWithoutZeroEndpoint = typename Slice<1, Length<InEndpointsList>::value - 1, InEndpointsList>::type;        
    public:
        /**
         * @brief Extends endpoint (init addresses)
         * 
         * @tparam Endpoint Endpoint to extend
         */
        template<typename Endpoint>
        using ExtendEndpoint = 
            typename Select<Endpoint::Direction == EndpointDirection::Bidirectional,
            BidirectionalEndpoint<
                Endpoint,
                EndpointInReg<TypeIndex<Endpoint, InEndpointsList>::value>, // IN register
                EndpointOutReg<TypeIndex<Endpoint, InEndpointsList>::value>, // OUT register
                static_cast<uint8_t>(TypeIndex<Endpoint, InEndpointsList>::value), // Fifo number
                FifoBaseAddress + TypeIndex<Endpoint, InEndpointsList>::value * EpFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename Select<Endpoint::Direction == EndpointDirection::Out,
            OutEndpoint<
                Endpoint,
                EndpointOutReg<Endpoint::Number>, // OUT register
                FifoBaseAddress + Endpoint::Number * EpFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename Select<Endpoint::Direction == EndpointDirection::In,
            InEndpoint<
                Endpoint,
                EndpointInReg<Endpoint::Number>, // IN register
                Endpoint::Number, // Fifo number
                FifoBaseAddress + Endpoint::Number * EpFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
        void>::value>::value>::value; // RxFifo (same for all endpoints)

        /**
         * @brief Inits USB PMA
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            OtgFsGlobal()->GRXFSIZ = RxFifoSize;

            (InitTransmitFifos<InEndpoints>(), ...);            
        }

    private:
        /**
         * @brief Init TX fifo for endpoint.
         * 
         * @details TX fifo size for endpoint = 2 x MaxPacketSize
         * 
         * @tparam Endpoint Endpoint
         */
        template<typename Endpoint>
        static void InitTransmitFifos()
        {
            if constexpr (Endpoint::Number == 0)
            {
                OtgFsGlobal()->DIEPTXF0_HNPTXFSIZ = (CalculateTxFifoDepth(Endpoint::MaxPacketSize) << 16) | RxFifoSize;
            }
            else
            {
                using InEndpointsBefore = Slice<0, TypeIndex<Endpoint, InEndpointsList>::value, InEndpointsList>::type;
                static const uint16_t FifoOffset = RxFifoSize + SumOfFifoSize<InEndpointsBefore>::value;
                OtgFsGlobal()->DIEPTXF[Endpoint::Number - 1] = (CalculateTxFifoDepth(Endpoint::MaxPacketSize) << 16) | FifoOffset;
            }
        }

        /**
         * @brief Calculates TX FIFO depth 
         * 
         * @details In default Tx buffer size is 2 * MaxPacketSize,
         * but minimum value IN endpoint TxFIFO depth is 16 (in terms of 32-bit words).
         * So, real size (in bytes) is Max(MaxPacketSize * 2, 64)
         * 
         * @param endpointMaxPacketSize Endpoint max packet size (in bytes)
         * @return constexpr uint16_t (in terms of 32-bit words)
         */
        static consteval uint32_t CalculateTxFifoDepth(uint16_t endpointMaxPacketSize)
        {
            return (2 * ((endpointMaxPacketSize + 3) / 4)) > 16
                ? (2 * ((endpointMaxPacketSize + 3) / 4))
                : 16;
        }

        /**
         * @brief Calculate sum of endpoints FIFO size.
         * 
         * @tparam Endpoints 
         */
        template<typename Endpoints>
        class SumOfFifoSize{};
        template<typename... Endpoints>
        class SumOfFifoSize<TypeList<Endpoints...>>
        {
        public:
            const static uint16_t value = (0 + ... + CalculateTxFifoDepth(Endpoints::MaxPacketSize));
        };
    };

    /**
     * @brief Endpoints manager.
     * 
     * @tparam Endpoints ALl endpoints list
     */
    template<typename Endpoints>
    using EndpointsManager = EndpointsManagerBase
    <
        SortedUniqueEndpoints<Endpoints>,
        Sample_t<IsInOrBidirectionalEndpoint, SortedUniqueEndpoints<Endpoints>>,
        Sample_t<IsOutOrBidirectionalEndpoint, SortedUniqueEndpoints<Endpoints>>
    >;
#endif
    /**
     * @brief Endpoints initalizer
     * 
     * @tparam Endpoints All endpoints list
     */
    template<typename... Endpoints>
    using EndpointsInitializer = EndpointsManager<TypeList<Endpoints...>>;

    /**
     * @brief Implements endpoint`s handlers management.
     * 
     * @tparam Endpoints Unique sorted endpoints.
     * @tparam Indexes Handlers indexes.
     */
    template<typename Endpoints, typename Indexes>
    class EndpointHandlersBase;
    template<typename... Endpoints, int8_t... Indexes>
    class EndpointHandlersBase<TypeList<Endpoints...>, Int8_tArray<Indexes...>>
    {
        static constexpr EpRequestHandler _handlers[] = {Endpoints::Handler...};
        static constexpr int8_t _handlersIndexes[] = {Indexes...};
    public:
        inline static void Handle(uint8_t number, EndpointDirection direction)
        {
            _handlers[_handlersIndexes[2 * number + (direction == EndpointDirection::Out ? 1 : 0)]]();
        }
    };
#if defined (USB_OTG_FS)
    /**
     * @brief Implements endpoint`s handlers management.
     * 
     * @tparam Endpoints Unique sorted endpoints.
     * @tparam Indexes Handlers indexes.
     */
    template<typename OutEndpoints, typename Indexes>
    class EndpointFifoNotEmptyHandlersBase;
    template<typename... OutEndpoints, int8_t... Indexes>
    class EndpointFifoNotEmptyHandlersBase<TypeList<OutEndpoints...>, Int8_tArray<Indexes...>>
    {
        /**
         * @todo Pack rx fifo handlers via add one In endpoints as template parameter.
         */
        static constexpr EpRxFifoNotEmptyHandler _rxFifoHandlers[] = {OutEndpoints::HandlerFifoNotEmpty...};
        static constexpr int8_t _handlersIndexes[] = {Indexes...};
    public:
        inline static void HandleRxFifoNotEmpty(uint8_t number, uint16_t size)
        {
            _rxFifoHandlers[_handlersIndexes[2 * number + 1]](size);
        }
    };
#endif
    /**
     * @brief Implements endpoint`s handlers indexes management.
     * 
     * @tparam Index Handler index.
     * @tparam Endpoints Unique sorted endpoints.
     */
    template<int8_t Index, typename Endpoints>
    class EndpointHandlersIndexes
    {
        using Predicate = Select<Index % 2 == 0, IsTxOrBidirectionalEndpointWithNumber<Index / 2>, IsRxOrBidirectionalEndpointWithNumber<Index / 2>>::value;
        static const int8_t EndpointIndex = Search<Predicate::template type, Endpoints>::value;
    public:
        using type = typename Int8_tArray_InsertBack<typename EndpointHandlersIndexes<Index - 1, Endpoints>::type, EndpointIndex>::type;
    };
    template<typename Endpoints>
    class EndpointHandlersIndexes<-1, Endpoints>
    {
    public:
        using type = Int8_tArray<>;
    };

    /**
     * @brief Max endpoint number
     * 
     * @tparam Endpoints Endpoints
     */
    template<typename Endpoints>
    const int8_t MaxEndpointNumber = GetType<Zhele::TemplateUtils::Length<SortedUniqueEndpoints<Endpoints>>::value - 1, SortedUniqueEndpoints<Endpoints>>::type::Number;

    /**
     * @brief Endpoint`s handlers.
     */
    template<typename Endpoints>
    using EndpointHandlers = EndpointHandlersBase<SortedUniqueEndpoints<Endpoints>,
        typename EndpointHandlersIndexes<MaxEndpointNumber<Endpoints> * 2 + 1, SortedUniqueEndpoints<Endpoints>>::type>;

#if defined (USB_OTG_FS)
    /**
     * @brief Endpoint`s RXFLVLM handlers.
     */
    template<typename Endpoints>
    using EndpointFifoNotEmptyHandlers = EndpointFifoNotEmptyHandlersBase<Sample_t<IsOutOrBidirectionalEndpoint, SortedUniqueEndpoints<Endpoints>>,
        typename EndpointHandlersIndexes<MaxEndpointNumber<Endpoints> * 2 + 1, Sample_t<IsOutOrBidirectionalEndpoint, SortedUniqueEndpoints<Endpoints>>>::type>;
#endif
}
#endif // ZHELE_USB_ENDPOINTS_MANAGER_H