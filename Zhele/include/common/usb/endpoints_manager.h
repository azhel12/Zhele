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
     * @brief Null endpoint (useful for search, sort)
     */
    class NullEndpoint
    {
    public:
        static const uint16_t Number = -1;
        static const EndpointDirection Direction = EndpointDirection::In;
        static const EndpointType Type = EndpointType::Control;
    };

    /**
     * @brief USB offsets calculator
     * 
     * @tparam Endpoints
    */
    template<typename... Endpoints>
    class OffsetCalculator
    {
        static constexpr auto _endpoints = TypeList<Endpoints...>{};
    public:
        /**
         * @brief Constexpr constructor (instead NTTP)
         * 
         * @param [in] endpoints Endpoints typelist object
        */
        constexpr OffsetCalculator(auto endpoints) {}

        /**
         * @brief Returns buffer offset for given endpoint
         * 
         * @param [in] endpoints Boxed endpoint
         * 
         * @returns Buffer offset for endpoint
        */
        static consteval auto GetBufferOffset(auto endpoint) {
            constexpr auto index = _endpoints.search(endpoint);

            if constexpr (index == 0) {
                return 0;
            } else {
                constexpr auto previousEndpoint = _endpoints.template get<index - 1>();

                return GetBufferOffset(previousEndpoint) + 
                    ((previousEndpoint.Type == EndpointType::BulkDoubleBuffered
                    || previousEndpoint.Direction == EndpointDirection::Bidirectional)
                        ? previousEndpoint.MaxPacketSize * 2
                        : previousEndpoint.MaxPacketSize);
            }
        }
        /**
         * @brief Template version of @ref GetBufferOffset method
        */
        template<typename Endpoint>
        static consteval auto GetBufferOffset() {
            return GetBufferOffset(TypeBox<Endpoint>{});
        }

        /**
         * @brief Returns packet descriptor offset for given endpoint
         * 
         * @param [in] endpoint Boxed endpoint
         * 
         * @returns Decriptor offset
        */
        static consteval auto GetPacketDescriptorOffset(auto endpoint) {
            constexpr auto index = _endpoints.search(endpoint)();

            if constexpr (index == 0) {
                return 0;
            } else {
                constexpr auto previousEndpoint = _endpoints.template get<index - 1>();
                return endpoint.Type == EndpointType::BulkDoubleBuffered
                    ? 4 + GetPacketDescriptorOffset(previousEndpoint)
                    : 2 + GetPacketDescriptorOffset(previousEndpoint);
            }
        }

        /**
         * @brief Template version of @ref GetPacketDescriptorOffset method
        */
        template<typename Endpoint>
        static consteval auto GetPacketDescriptorOffset() {
            return GetPacketDescriptorOffset(TypeBox<Endpoint>{});
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    OffsetCalculator(TypeList<Endpoints...> endpoints) -> OffsetCalculator<Endpoints...>;

#if defined (USB)
    /**
     * @brief Calculates endpoint`s registers.
     * 
     * @details Endpoints can have any address 0..15, but MCU contains only 8 registers.
     * So, you can declare 3 endpoints with numbers 0, 6, 10 (for example) and this class
     * assigns EP0R to Ep0, EP1R to Ep6 and EP2R to Ep10
     * 
     * @tparam Endpoints Endpoints
     */
    template<typename... Endpoints>
    class EndpointRegistersManager
    {
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
        
        static constexpr auto _endpointRegs = TypeList<Ep0Reg, Ep1Reg, Ep2Reg, Ep3Reg, Ep4Reg, Ep5Reg, Ep6Reg, Ep7Reg>{};
        static constexpr auto _endpoints = TypeList<Endpoints...>{};
    
    public:
        /**
         * @brief Constexpr constructor for CTAD
        */
        constexpr EndpointRegistersManager(auto endpoints) {}

        /**
         * @brief Returns register number for given endpoint (EPRn)
         * 
         * @param [in] endpoint Boxed endpoint
         * 
         * @returns Endpoint EPR number
        */
        static consteval auto GetRegisterNumber(auto endpoint) {
            constexpr auto index = _endpoints.search(endpoint);

            if constexpr (index == 0) {
                return 0;
            } else {
                constexpr auto previousEndpoint = _endpoints.template get<index - 1>();

                constexpr bool IsEndpointIncompatibleWithPrevious = endpoint.Number == previousEndpoint.Number &&
                (endpoint.Type == EndpointType::Control
                    || endpoint.Type == EndpointType::BulkDoubleBuffered
                    || endpoint.Direction == EndpointDirection::Bidirectional
                    || previousEndpoint.Type == EndpointType::Control
                    || previousEndpoint.Type == EndpointType::BulkDoubleBuffered
                    || previousEndpoint.Direction == EndpointDirection::Bidirectional);

                static_assert(!IsEndpointIncompatibleWithPrevious, "Incompatible endpoints with same number");

                return endpoint.Number == previousEndpoint.Number
                    ? GetRegisterNumber(previousEndpoint)
                    : GetRegisterNumber(previousEndpoint) + 1;
            }
        }

        /**
         * @brief Template variant of @ref GetRegisterNumber method
        */
        template<typename Endpoint>
        static consteval auto GetRegisterNumber() {
            return GetRegisterNumber(TypeBox<Endpoint>{});
        }

        /**
         * @brief Returns boxed endpoint reg for given endpoint
         * 
         * @tparam Endpoint Endpoint
         * 
         * @returns Boxed EPRn
        */
        template<typename Endpoint>
        static consteval auto GetEndpointReg()
        {
            return _endpointRegs.template get<GetRegisterNumber(TypeBox<Endpoint>{})>();
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    EndpointRegistersManager(TypeList<Endpoints...> endpoints) -> EndpointRegistersManager<Endpoints...>;

    /**
     * @brief Implements endpoint`s buffers management.
     * 
     * @tparam Endpoints All endpoints
     */

    template<typename... Endpoints>
    class EndpointsManager
    {
        /// USB PMA base address
        static constexpr uint32_t PmaBufferBase = USB_PMAADDR;
        /// All endpoints sorted by number
        static constexpr auto _sortedUniqueEndpoints = TypeList<Endpoints...>{}.remove_duplicates().sort([](auto first, auto second){ return first.Number < second.Number; });
        /// EPRn manager
        static constexpr auto _registersManager = EndpointRegistersManager{_sortedUniqueEndpoints};
        /// Buffers and registers offset calculator
        static constexpr auto _offsetCalculator = OffsetCalculator{_sortedUniqueEndpoints};
        /// Buffer descriptor table size (all realy used endpoints * 8)
        static constexpr auto BdtSize = 8 * (_registersManager.GetRegisterNumber(_sortedUniqueEndpoints.back()) + 1);

        /**
         * @brief Returns buffer offset for given endpoint
         * 
         * @param [in] endpoint Boxed endpoint
         * 
         * @returns Buffer offset
        */
        static consteval auto GetBufferOffset(auto endpoint) {
            return BdtSize + _offsetCalculator.GetBufferOffset(endpoint);
        }
        /// @brief Template variant of @ref GetBufferOffset
        template<typename Endpoint>
        static constexpr uint32_t BufferOffset = GetBufferOffset(TypeBox<Endpoint>{});

        /**
         * @brief Returns BDT cell offset for given endpoint
         * 
         * @param [in] endpoint Boxed endpoint
         * 
         * @returns BDT offset
        */
        static consteval auto GetBdtCellOffset(auto endpoint) {
            return _registersManager.GetRegisterNumber(endpoint) * 8
                + (endpoint.Type == EndpointType::BulkDoubleBuffered
                || endpoint.Direction == EndpointDirection::In
                || endpoint.Direction == EndpointDirection::Bidirectional
                    ? 0
                    : 4);
        }
        /// @brief Template variant of @ref GetBdtCellOffset
        template<typename Endpoint>
        static const uint32_t BdtCellOffset = GetBdtCellOffset(TypeBox<Endpoint>{});

        /// @brief BDT base address
        static const uint32_t BdtBase = PmaBufferBase;

    public:
        /**
         * @brief Constexpr constructor for CTAD
        */
        constexpr EndpointsManager(auto endpoints) {}

        /**
         * @brief Extends endpoint (init addresses)
         * 
         * @tparam Endpoint Endpoint to extend
         */
        template<typename Endpoint>
        using ExtendEndpoint = 
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Bidirectional,
                BidirectionalEndpoint<Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // TxBuffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // TxCount
                    PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + Endpoint::MaxPacketSize), // RxBuffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //RxCount
            typename std::conditional_t<Endpoint::Type == EndpointType::BulkDoubleBuffered,
                BulkDoubleBufferedEndpoint<Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer0
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // Buffer0Count
                    PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + Endpoint::MaxPacketSize), // Buffer1
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //Buffer1Count
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::In,
                InEndpoint<Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)>, // BufferCount
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Out,
                OutEndpoint<Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)>, // BufferCount
            void>>>>;

        /**
         * @brief Inits USB PMA
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            InitTxFieldsInDescriptor();
            InitRxAddressFieldInDescriptor();
            InitRxCountFieldInDescriptor();
            InitSecondRxCountFieldInDescriptor();
        }

    private:
        static void InitTxFieldsInDescriptor()
        {
            _sortedUniqueEndpoints.foreach([](auto endpoint) {
                *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * GetBdtCellOffset(endpoint)) = GetBufferOffset(endpoint);
                *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 2)) = 0u;
            });
        }
        
        static void InitRxAddressFieldInDescriptor()
        {
            constexpr auto bidirectionalAndBulkDoubleBufferedEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint){
                return endpoint.Direction == EndpointDirection::Bidirectional || endpoint.Type == EndpointType::BulkDoubleBuffered;
            });

            bidirectionalAndBulkDoubleBufferedEndpoints.foreach([](auto endpoint){
                *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 4)) = GetBufferOffset(endpoint) + endpoint.MaxPacketSize;
            });
        }
        
        static void InitRxCountFieldInDescriptor()
        {
            constexpr auto outEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint){
                return endpoint.Direction == EndpointDirection::Out
                    && endpoint.Type != EndpointType::Control
                    && endpoint.Type != EndpointType::ControlStatusOut;
            });

            outEndpoints.foreach([](auto endpoint){
                *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 2)) = CalculateRxCountValue(endpoint);
            });
        }

        static void InitSecondRxCountFieldInDescriptor()
        {
            constexpr auto bidirectionalAndBulkDoubleBufferedEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint){
                return endpoint.Direction == EndpointDirection::Bidirectional || endpoint.Type == EndpointType::BulkDoubleBuffered;
            });

            bidirectionalAndBulkDoubleBufferedEndpoints.foreach([](auto endpoint) {
                *reinterpret_cast<uint16_t*>(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 6)) = CalculateRxCountValue(endpoint);
            });
        }

        static consteval uint16_t CalculateRxCountValue(auto endpoint)
        {
            return endpoint.MaxPacketSize <= 62
                ? (endpoint.MaxPacketSize / 2) << 10
                : 0x8000 | ((endpoint.MaxPacketSize / 32 - 1) << 10);
        }
    };

#elif defined (USB_OTG_FS)
    /**
     * @brief Calculates endpoint`s registers.
     * 
     * @details Endpoints can have any address 0..15, but MCU contains only 8 registers.
     * So, you can declare 3 endpoints with numbers 0, 6, 10 (for example) and this class
     * assigns EP0R to Ep0, EP1R to Ep6 and EP2R to Ep10
     * 
     * @tparam Endpoints Endpoints
     */
    template<typename... Endpoints>
    class EndpointRegistersManager
    {
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

        static constexpr auto _endpointInRegs = Zhele::TemplateUtils::TypeList<InEp0Reg, InEp1Reg, InEp2Reg, InEp3Reg>{};
        static constexpr auto _endpointOutRegs = Zhele::TemplateUtils::TypeList<OutEp0Reg, OutEp1Reg, OutEp2Reg, OutEp3Reg>{};

    public:
        /**
         * @brief Constexpr constructor for CTAD
        */
        constexpr EndpointRegistersManager(auto endpoints) {}

        /**
         * @brief Returns boxed endpoint reg for given IN endpoint
         * 
         * @tparam Endpoint Endpoint
         * 
         * @returns Boxed EPRn
        */
        template<int Number>
        static consteval auto GetEndpointInReg() {
            if constexpr (Number == -1)
                return TypeBox<void>{};
            else
                return _endpointInRegs.template get<Number>();
        }

        /**
         * @brief Returns boxed endpoint reg for given OUT endpoint
         * 
         * @tparam Endpoint Endpoint
         * 
         * @returns Boxed EPRn
        */
        template<int Number>
        static consteval auto GetEndpointOutReg() {
            if constexpr (Number == -1)
                return TypeBox<void>{};
            else
                return _endpointOutRegs.template get<Number>();
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    EndpointRegistersManager(TypeList<Endpoints...> endpoints) -> EndpointRegistersManager<Endpoints...>;

    template<typename... Endpoints>
    class EndpointsManager
    {
        IO_STRUCT_WRAPPER(USB_OTG_FS, OtgFsGlobal, USB_OTG_GlobalTypeDef);
        /// All endpoints sorted by number
        static constexpr auto _sortedUniqueEndpoints = TypeList<Endpoints...>{}.remove_duplicates().sort([](auto first, auto second){ return first.Number < second.Number; });
        /// Out or bidirectional endpoints sorted by number
        static constexpr auto _sortedUniqueOutEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint) {
            return endpoint.Direction == EndpointDirection::Out || endpoint.Direction == EndpointDirection::Bidirectional;
        });
        /// In or bidirectional endpoints sorted by number
        static constexpr auto _sortedUniqueInEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint) {
            return endpoint.Direction == EndpointDirection::In || endpoint.Direction == EndpointDirection::Bidirectional;
        });
        /// EPRn manager
        static constexpr auto _registersManager = EndpointRegistersManager{_sortedUniqueEndpoints};
        /// Buffers and registers offset calculator
        static constexpr auto _offsetCalculator = OffsetCalculator{_sortedUniqueOutEndpoints};
        /// FIFO base address
        static constexpr uint32_t _fifoBaseAddress = USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE;
        /// FIFO size for one endpoint
        static constexpr uint32_t _epFifoSize = USB_OTG_FIFO_SIZE;

        /**
         * @brief Returns buffer offset for given endpoint
         * 
         * @param [in] endpoint Boxed endpoint
         * 
         * @returns Buffer offset
        */
        static consteval auto GetBufferOffset(auto endpoint) {
            return _offsetCalculator.GetBufferOffset(endpoint);
        }

        /// @brief Template variant of @ref GetBufferOffset
        template<typename Endpoint>
        static constexpr uint32_t BufferOffset =  GetBufferOffset(TypeBox<Endpoint>{});

    public:
        /**
         * @brief Constexpr constructor for CTAD
        */
        constexpr EndpointsManager(auto endpoints) {}

        /**
         * @brief Extends endpoint (init addresses)
         * 
         * @tparam Endpoint Endpoint to extend
         */
        template<typename Endpoint>
        using ExtendEndpoint = 
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Bidirectional,
                BidirectionalEndpoint<
                    Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointInReg<Endpoint::Number>()>, // IN register
                    TypeUnbox<_registersManager.template GetEndpointOutReg<Endpoint::Number>()>, // OUT register
                    static_cast<uint8_t>(_sortedUniqueInEndpoints.template search<Endpoint>()), // Fifo number
                    _fifoBaseAddress + Endpoint::Number * _epFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Out,
                OutEndpoint<
                    Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointOutReg<Endpoint::Number>()>, // OUT register
                    _fifoBaseAddress + Endpoint::Number * _epFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::In,
                InEndpoint<
                    Endpoint,
                    TypeUnbox<_registersManager.template GetEndpointInReg<Endpoint::Number>()>, // IN register
                    Endpoint::Number, // Fifo number
                    _fifoBaseAddress + Endpoint::Number * _epFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
        void>>>;

        /**
         * @brief Inits USB PMA
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            OtgFsGlobal()->GRXFSIZ = GetRxFifoSize();

            InitTransmitFifos();
        }

    private:
        /**
         * @brief Init TX fifo for endpoint.
         * 
         * @details TX fifo size for endpoint = 2 x MaxPacketSize
         * 
         * @tparam Endpoint Endpoint
         */
        static void InitTransmitFifos()
        {
            _sortedUniqueInEndpoints.foreach([](auto endpoint){
                if constexpr (endpoint.Number == 0) {
                    OtgFsGlobal()->DIEPTXF0_HNPTXFSIZ = (CalculateTxFifoDepth(endpoint) << 16) | GetRxFifoSize();
                } else {
                    constexpr auto inEndpointsWithLessNumber = _sortedUniqueInEndpoints.filter([endpoint](auto other) {
                        return other.Number < endpoint.Number;
                    });
                    constexpr uint16_t fifoOffset = GetRxFifoSize() + GetSumOfFifoSize(inEndpointsWithLessNumber);
                    OtgFsGlobal()->DIEPTXF[endpoint.Number - 1] = (CalculateTxFifoDepth(endpoint) << 16) | fifoOffset;    
                }
            });
        }

        /**
         * @brief Calculates the largest of max packet size for endpoints list
         * 
         * @param [in] endpoints Endpoints list
         * 
         * @returnsThe larges of max packet sizes
        */
        static consteval uint16_t GetLargestEndpointsMaxPacketSizeMax(auto endpoints)
        {
            uint16_t maxPacketSize = 0;

            endpoints.foreach([&maxPacketSize](auto endpoint) {
                if (endpoint.MaxPacketSize > maxPacketSize) {
                    maxPacketSize = endpoint.MaxPacketSize;
                }
            });

            return maxPacketSize;
        }

        /**
         * @brief Calculates TX FIFO depth 
         * 
         * @details In default Tx buffer size is 2 * MaxPacketSize,
         * but minimum value IN endpoint TxFIFO depth is 16 (in terms of 32-bit words).
         * So, real size (in bytes) is Max(MaxPacketSize * 2, 64)
         * 
         * @param endpoint Boxed endpoint
         * 
         * @return constexpr uint16_t (in terms of 32-bit words)
         */
        static consteval uint32_t CalculateTxFifoDepth(auto endpoint)
        {
            return (2 * ((endpoint.MaxPacketSize + 3) / 4)) > 16
                ? (2 * ((endpoint.MaxPacketSize + 3) / 4))
                : 16;
        }

        /**
         * @brief Calculate sum of endpoints FIFO size.
         * 
         * @param [in] endpoints Endpoints list
         * 
         * @returns Sum of fifo size
         */
        static consteval uint16_t GetSumOfFifoSize(auto endpoints)
        {
            uint16_t result = 0;

            endpoints.foreach([&result](auto endpoint) {
                result += CalculateTxFifoDepth(endpoint);
            });

            return result;
        }

        /**
         * @brief Returns receive FIFO size
         * 
         * @returns Rx FIFO size
        */
        static consteval uint16_t GetRxFifoSize()
        {
            return (11 + 2 * ((3 + GetLargestEndpointsMaxPacketSizeMax(_sortedUniqueOutEndpoints)) / 4)) > 16 // 11 = 10 (SETUP) + 1 (global out NAK)
            ? 11 + 2 * ((3 + GetLargestEndpointsMaxPacketSizeMax(_sortedUniqueOutEndpoints)) / 4)
            : 16;
        }
    };
#endif
    template<typename... Ep>
    EndpointsManager(TypeList<Ep...> endpoints) -> EndpointsManager<Ep...>;

    /**
     * @brief Endpoints initalizer
     * 
     * @tparam Endpoints All endpoints list
     */
    template<typename... Endpoints>
    using EndpointsInitializer = EndpointsManager<Endpoints...>;

    /**
     * @brief Implements endpoint`s handlers management.
     * 
     * @tparam Endpoints All endpoints.
     */
    template<typename... Endpoints>
    class EndpointHandlers
    {
        /**
         * @brief Builds indexes array for given interfaces
         * 
         * @details I have been use TypeUnbox<> for parameters because on element of Endpoints
         *  is Typebox without inherit - it is TypeBox<DeviceBase<XXX>>, where DeviceBase<XXX> is incomplete type. 
         * 
         * @returns std::array with indexes
        */
        static consteval auto BuildIndexesArray()
        {
            constexpr auto endpoints = TypeList<Endpoints...>{};
            constexpr auto maxEndpointsNumber = TypeUnbox<endpoints.sort([](auto a, auto b) {
                return TypeUnbox<a>::Number < TypeUnbox<b>::Number;
            }).back()>::Number;

            std::array<int8_t, (maxEndpointsNumber + 1) * 2> indexes {};

            for(auto& i : indexes) {
                i = -1;
            }

            endpoints.foreach([&indexes, i{0}](auto endpoint) mutable {
                if (TypeUnbox<endpoint>::Direction == EndpointDirection::In || TypeUnbox<endpoint>::Direction == EndpointDirection::Bidirectional)
                    indexes[2 * TypeUnbox<endpoint>::Number] = i;
                if (TypeUnbox<endpoint>::Direction == EndpointDirection::Out || TypeUnbox<endpoint>::Direction == EndpointDirection::Bidirectional)
                    indexes[1 + 2 * TypeUnbox<endpoint>::Number] = i;
                ++i;
            });

            return indexes;
        }

        static constexpr EpRequestHandler _handlers[] = {Endpoints::Handler...};
        static constexpr auto _handlersIndexes = BuildIndexesArray();
    public:

        template<typename... Ep>
        constexpr EndpointHandlers(TypeList<Ep...> endpoints) {}

        inline static void Handle(uint8_t number, EndpointDirection direction)
        {
            _handlers[_handlersIndexes[2 * number + (direction == EndpointDirection::Out ? 1 : 0)]]();
        }
    };
    /// deduction guid for EndpointHandlers
    template<typename... Ep>
    EndpointHandlers(TypeList<Ep...> endpoints) -> EndpointHandlers<Ep...>;

#if defined (USB_OTG_FS)
    /**
     * @brief Implements endpoint`s handlers management.
     * 
     * @tparam Endpoints Unique sorted endpoints.
     * @tparam Indexes Handlers indexes.
     */
    template<typename... OutEndpoints>
    class EndpointFifoNotEmptyHandlers
    {
        /**
         * @brief Builds indexes array for given endpoints
         * 
         * @returns std::array with indexes
        */
        static consteval auto BuildIndexesArray()
        {
            constexpr auto outEndpoints = TypeList<OutEndpoints...>{};

            constexpr auto maxEndpointNumber = outEndpoints.sort([](auto a, auto b) { return TypeUnbox<a>::Number < TypeUnbox<b>::Number; }).back().Number;
            std::array<int8_t, maxEndpointNumber + 1> indexes {};

            for(auto& i : indexes) {
                i = -1;
            }

            outEndpoints.foreach([&indexes, i{0}](auto endpoint) mutable {
                indexes[TypeUnbox<endpoint>::Number] = i++;
            });

            return indexes;
        }

        /**
         * @todo Pack rx fifo handlers via add one In endpoints as template parameter.
         */
        static constexpr EpRxFifoNotEmptyHandler _rxFifoHandlers[] = {OutEndpoints::HandlerFifoNotEmpty...};
        static constexpr auto _handlersIndexes = BuildIndexesArray();

    public:
        /**
         * @brief Constexpr constructor for CTAD
        */
        constexpr EndpointFifoNotEmptyHandlers(auto endpoints) {}

        inline static void HandleRxFifoNotEmpty(uint8_t number, uint16_t size)
        {
            _rxFifoHandlers[_handlersIndexes[number]](size);
        }
    };
    /// deduction guid for EndpointHandlers
    template<typename... Ep>
    EndpointFifoNotEmptyHandlers(TypeList<Ep...> endpoints) -> EndpointFifoNotEmptyHandlers<Ep...>;
#endif
}
#endif // ZHELE_USB_ENDPOINTS_MANAGER_H