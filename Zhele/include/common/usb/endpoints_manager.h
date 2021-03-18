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
     * @brief Predicat for search Tx endpoint by number
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
     * @brief Predicat for search Rx endpoint by number
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
     * @brief Predicat for search Tx or bidirectional endpoint by number
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
            static const bool value = (Endpoint::Number == Number && Endpoint::Direction == EndpointDirection::Out || Endpoint::Direction == EndpointDirection::Bidirectional);
        };
    };

    /**
     * @brief Predicat for search Rx or bidirectional endpoint by number
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
            static const bool value = (Endpoint::Number == Number && Endpoint::Direction == EndpointDirection::In || Endpoint::Direction == EndpointDirection::Bidirectional);
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
     */
    template<typename Endpoint>
    class IsBidirectionalOrBulkDoubleBufferedEndpoint
    {
    public:
        static const bool value = (Endpoint::Type == EndpointType::BulkDoubleBuffered
                                || Endpoint::Direction == EndpointDirection::Bidirectional);
    };

    /**
     * @brief Predicate for search Rx (no control) endpoints/
     */
    template<typename Endpoint>
    class IsOutEndpoint
    {
    public:
        static const bool value = (Endpoint::Direction == EndpointDirection::Out
                                && Endpoint::Type != EndpointType::Control
                                && Endpoint::Type != EndpointType::ControlStatusOut
                                && Endpoint::Type != EndpointType::BulkDoubleBuffered);
    };

    /**
     * @brief Predicate for search Tx double-buffered
     */
    template<typename Endpoint>
    class IsBulkDoubleBufferedTxEndpoint
    {
    public:
        static const bool value = (Endpoint::Type == EndpointType::BulkDoubleBuffered && Endpoint::Direction == EndpointDirection::Out);
    };

    /**
     * @brief Sort endpoints by number and direction
     */
    template <typename T, typename U>
    struct NumberAndDirectionComparator : std::conditional_t<(U::Number < T::Number || (U::Number == T::Number && static_cast<uint8_t>(U::Direction) > static_cast<uint8_t>(T::Direction))), std::true_type, std::false_type>
    {};
    template<typename Endpoints>
    using EndpointsSortedByNumberAndDirection = typename TypeListSort<NumberAndDirectionComparator, Endpoints>::type;

    /**
     * @brief Unique endpoints sorted by numbers/direction.
     */
    template<typename Endpoints>
    using SortedUniqueEndpoints = EndpointsSortedByNumberAndDirection<typename Unique<Endpoints>::type>;

    /**
     * @brief Calculates endpoint`s buffers offsets.
     */
    template<int Index, typename>
    class OffsetOfBuffer;
    template<typename... Endpoints>
    class OffsetOfBuffer<0, TypeList<Endpoints...>>
    {
    public:
        static const unsigned value = 0;
    };
    template<int Number, typename... Endpoints>
    class OffsetOfBuffer<Number, TypeList<Endpoints...>>
    {
        using Endpoint = GetType<Number, TypeList<Endpoints...>>::type;
        using PreviousEndpoint = GetType<Number - 1, TypeList<Endpoints...>>::type;
    public:
        static const uint16_t value = OffsetOfBuffer<Number - 1, TypeList<Endpoints...>>::value +
            (PreviousEndpoint::Type == EndpointType::BulkDoubleBuffered
            || PreviousEndpoint::Type == EndpointType::Control
            || PreviousEndpoint::Type == EndpointType::ControlStatusOut
            || PreviousEndpoint::Direction == EndpointDirection::Bidirectional)
                ? PreviousEndpoint::MaxPacketSize * 2
                : PreviousEndpoint::MaxPacketSize;
    };

    /**
     * @brief Calculates endpoint`s bdt cells.
     */
    template<int Index, typename>
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

    template<uint8_t _EndpointNumber>
    using EndpointReg = Zhele::TemplateUtils::GetType<_EndpointNumber, EndpointRegs>::type;

    static const uint32_t PmaBufferBase = USB_PMAADDR;

    class NullEndpoint
    {
    public:
        static const uint16_t Number = -1;
        static const EndpointDirection Direction = EndpointDirection::In;
        static const EndpointType Type = EndpointType::Control;
    };
    /**
     * @brief Calculates endpoint`s registers.
     * Endpoints can have any address 0..15, but MCU contains only 8 registers.
     */
    template<typename...>
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
     */
    template<typename...>
    class EndpointsManagerBase;
    template<typename... AllEndpoints, typename... BidirectionalAndBulkDoubleBufferedEndpoints, typename... RxEndpoints, typename... BulkDoubleBufferedTxEndpoints>
    class EndpointsManagerBase<TypeList<AllEndpoints...>, TypeList<BidirectionalAndBulkDoubleBufferedEndpoints...>, TypeList<RxEndpoints...>, TypeList<BulkDoubleBufferedTxEndpoints...>>
    {
        using AllEndpointsList = TypeList<AllEndpoints...>;

        /// Buffer descriptor table size (all realy used endpoints * 8)
        static const auto BdtSize = 8 * (EndpointEPRn<GetType_t<sizeof...(AllEndpoints) - 1, AllEndpointsList>, AllEndpointsList>::RegisterNumber + 1);

        /// Buffer offset in PMA for endpoint
        template<typename Endpoint>
        static constexpr uint32_t BufferOffset = BdtSize + OffsetOfBuffer<TypeIndex<Endpoint, AllEndpointsList>::value, AllEndpointsList>::value;

        /// Buffer descriptor offset in BDT for endpoint
        template<typename Endpoint>
        static constexpr uint32_t BdtCellOffset =
            EndpointEPRn<Endpoint, AllEndpointsList>::RegisterNumber * 8
                                + (Endpoint::Type == EndpointType::Control
                                || Endpoint::Type == EndpointType::ControlStatusOut
                                || Endpoint::Type == EndpointType::BulkDoubleBuffered
                                || Endpoint::Direction == EndpointDirection::Out
                                || Endpoint::Direction == EndpointDirection::Bidirectional
                                    ? 0
                                    : 4);
        /// BDT base
        static const uint32_t BdtBase = PmaBufferBase;
    public:
        /// Extends endpoint (init addresses)
        template<typename Endpoint>
        using ExtendEndpoint = 
            typename Select<Endpoint::Type == EndpointType::Control || Endpoint::Type == EndpointType::ControlStatusOut,
            ControlEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + BufferOffset<Endpoint>, // TxBuffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 2, // TxCount
                PmaBufferBase + BufferOffset<Endpoint> + Endpoint::MaxPacketSize, // RxBuffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 6>, //RxCount
            typename Select<Endpoint::Direction == EndpointDirection::Bidirectional,
            BidirectionalEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + BufferOffset<Endpoint>, // TxBuffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 2, // TxCount
                PmaBufferBase + BufferOffset<Endpoint> + Endpoint::MaxPacketSize, // RxBuffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 6>, //RxCount
            typename Select<Endpoint::Type == EndpointType::BulkDoubleBuffered,
            BulkDoubleBufferedEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + BufferOffset<Endpoint>, // Buffer0
                PmaBufferBase + BdtCellOffset<Endpoint> + 2, // Buffer0Count
                PmaBufferBase + BufferOffset<Endpoint> + Endpoint::MaxPacketSize, // Buffer1
                PmaBufferBase + BdtCellOffset<Endpoint> + 6>, //Buffer1Count
            typename Select<Endpoint::Direction == EndpointDirection::In,
            InEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + BufferOffset<Endpoint>, // Buffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 2>, // BufferCount
            typename Select<Endpoint::Direction == EndpointDirection::Out,
            OutEndpoint<Endpoint,
                typename EndpointEPRn<Endpoint, TypeList<AllEndpoints...>>::type,
                PmaBufferBase + BufferOffset<Endpoint>, // Buffer
                PmaBufferBase + BdtCellOffset<Endpoint> + 2>, // BufferCount
            void>::value>::value>::value>::value>::value;

        static void Init()
        {
            memset(reinterpret_cast<void*>(BdtBase), 0x00, BdtSize);
            // Init all endpoints offsets (and TX buffers for control and double-buffered bulk endpoints)
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<AllEndpoints>)) = BufferOffset<AllEndpoints>), ...);
            // Init RX buffer for control and bidirectional endpoints and second half-buffer for double-buffered bulk endpoints
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<BidirectionalAndBulkDoubleBufferedEndpoints> + 4)) = (BufferOffset<BidirectionalAndBulkDoubleBufferedEndpoints> + BidirectionalAndBulkDoubleBufferedEndpoints::MaxPacketSize)), ...);
            // Init RX_count buffer for RX endpoints
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<RxEndpoints> + 2)) = (RxEndpoints::MaxPacketSize <= 62
                                                                                            ? (RxEndpoints::MaxPacketSize / 2) << 10
                                                                                            : 0x8000 | (RxEndpoints::MaxPacketSize / 32) << 10)), ...);
            // Init RX_count buffer for control and bidirectional endpoints and second half-buffer for double-buffered bulk endpoints
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<BidirectionalAndBulkDoubleBufferedEndpoints> + 6)) = (BidirectionalAndBulkDoubleBufferedEndpoints::MaxPacketSize <= 62
                                                                                            ? (BidirectionalAndBulkDoubleBufferedEndpoints::MaxPacketSize / 2) << 10
                                                                                            : 0x8000 | (BidirectionalAndBulkDoubleBufferedEndpoints::MaxPacketSize / 32) << 10)), ...);

            // Set TX_count to 0 for TX double-buffered bulk
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<BulkDoubleBufferedTxEndpoints> + 2)) = 0), ...);
            ((*(reinterpret_cast<uint16_t*>(BdtBase + BdtCellOffset<BulkDoubleBufferedTxEndpoints> + 6)) = 0), ...);
        }
    };
    
    template<typename Endpoints>
    using EndpointsManager = EndpointsManagerBase<SortedUniqueEndpoints<Endpoints>,
        typename Sample<IsBidirectionalOrBulkDoubleBufferedEndpoint, SortedUniqueEndpoints<Endpoints>>::type,
        typename Sample<IsOutEndpoint, SortedUniqueEndpoints<Endpoints>>::type,
        typename Sample<IsBulkDoubleBufferedTxEndpoint, SortedUniqueEndpoints<Endpoints>>::type>;

    template<typename... Endpoints>
    using EndpointsInitializer = EndpointsManagerBase<SortedUniqueEndpoints<TypeList<Endpoints...>>,
        TypeList<>,
        TypeList<>,
        TypeList<>>;

    /**
     * @brief Static array of int8_t.
     */
    template<int8_t... Numbers>
    class Int8_tArray;
    template<typename, int8_t>
    /**
     * @brief Inserts value in back.
     */
    class Int8_tArray_InsertBack {};
    template<int8_t Value, int8_t... Numbers>
    class Int8_tArray_InsertBack<Int8_tArray<Numbers...>, Value>
    {
    public:
        using type = Int8_tArray<Numbers..., Value>;
    };

    /**
     * @brief Implements endpoint`s handlers management.
     * 
     * @tparam Endpoints Unique sorted endpoints.
     * @tparam Indexes Handlers indexes.
     */
    template<typename...>
    class EndpointHandlersBase;
    template<typename... Endpoints, int8_t... Indexes>
    class EndpointHandlersBase<TypeList<Endpoints...>, Int8_tArray<Indexes...>>
    {
        static constexpr EpRequestHandler _handlers[] = {Endpoints::Handler...};
        static constexpr int8_t _handlersIndexes[] = {Indexes...};
    public:
        inline static void Handle(uint8_t number, EndpointDirection direction)
        {
            _handlers[_handlersIndexes[number + (direction == EndpointDirection::Out ? 1 : 0)]]();
        }
    };

    /**
     * @brief Implements endpoint`s handlers indexes management.
     * 
     * @tparam Index Handler index.
     * @tparam Endpoints Unique sorted endpoints.
     */
    template<int8_t, typename...>
    class EndpointHandlersIndexes;
    template<typename... Endpoints>
    class EndpointHandlersIndexes<-1, TypeList<Endpoints...>>
    {
    public:
        using type = Int8_tArray<>;
    };
    template<int8_t Index, typename... Endpoints>
    class EndpointHandlersIndexes<Index, TypeList<Endpoints...>>
    {
        static const int temp = Zhele::TemplateUtils::Length<TypeList<Endpoints...>>::value - 1;

        using Predicate = Select<Index % 2 == 0, IsTxOrBidirectionalEndpointWithNumber<Index / 2>, IsRxOrBidirectionalEndpointWithNumber<Index / 2>>::value;
        static const int8_t EndpointIndex = Search<Predicate::template type, TypeList<Endpoints...>>::value;
    public:
        using type = typename Int8_tArray_InsertBack<typename EndpointHandlersIndexes<Index - 1, TypeList<Endpoints...>>::type, EndpointIndex>::type;
    };

    /**
     * @brief Endpoint`s handlers.
     */
    template<typename Endpoints>
    using EndpointHandlers = EndpointHandlersBase<SortedUniqueEndpoints<Endpoints>,
        typename EndpointHandlersIndexes<GetType<Zhele::TemplateUtils::Length<SortedUniqueEndpoints<Endpoints>>::value - 1, SortedUniqueEndpoints<Endpoints>>::type::Number * 2 - 1, SortedUniqueEndpoints<Endpoints>>::type>;
}
#endif // ZHELE_USB_ENDPOINTS_MANAGER_H