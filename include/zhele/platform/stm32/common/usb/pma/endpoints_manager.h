/**
 * @file
 * Implement endpoint`s registers and packet memory allocation for PMA-based device controller
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINTS_MANAGER_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINTS_MANAGER_H

#include "endpoint.h"
#include "../../ioreg.h"

#include <zhele/common/template_utils/type_list.h>

#include <type_traits>
#include <stdint.h>

namespace Zhele::Usb
{
    /**
     * @brief USB offsets calculator
     * 
     * @tparam Endpoints
    */
    template<typename... Endpoints>
    class OffsetCalculator
    {
        static constexpr auto _endpoints = template_utils::type_list<Endpoints...>{};
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
                        ? PmaBufferSize(previousEndpoint.MaxPacketSize) * 2
                        : PmaBufferSize(previousEndpoint.MaxPacketSize));
            }
        }
        /**
         * @brief Template version of @ref GetBufferOffset method
        */
        template<typename Endpoint>
        static consteval auto GetBufferOffset() {
            return GetBufferOffset(template_utils::type_box<Endpoint>{});
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
            return GetPacketDescriptorOffset(template_utils::type_box<Endpoint>{});
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    OffsetCalculator(template_utils::type_list<Endpoints...> endpoints) -> OffsetCalculator<Endpoints...>;

    /**
     * @brief Calculates endpoint`s registers
     * 
     * @details Endpoints can have any address 0..15, but MCU contains only 8 registers
     * So, you can declare 3 endpoints with numbers 0, 6, 10 (for example) and this class
     * assigns EP0R to Ep0, EP1R to Ep6 and EP2R to Ep10
     * 
     * @tparam Endpoints Endpoints
     */
    template<typename... Endpoints>
    class EndpointRegistersManager
    {
        /// Wrappers for EPnR registers
        /// C++ doesn't allows reinterpret_cast in compile-time
#if defined (ZHELE_USB_DRD)
        IO_REG_WRAPPER(USB_DRD_FS->CHEP0R, Ep0Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP1R, Ep1Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP2R, Ep2Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP3R, Ep3Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP4R, Ep4Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP5R, Ep5Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP6R, Ep6Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB_DRD_FS->CHEP7R, Ep7Reg, EndpointRegisterType);
#else
        IO_REG_WRAPPER(USB->EP0R, Ep0Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP1R, Ep1Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP2R, Ep2Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP3R, Ep3Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP4R, Ep4Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP5R, Ep5Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP6R, Ep6Reg, EndpointRegisterType);
        IO_REG_WRAPPER(USB->EP7R, Ep7Reg, EndpointRegisterType);
#endif
        
        static constexpr auto _endpointRegs = template_utils::type_list<Ep0Reg, Ep1Reg, Ep2Reg, Ep3Reg, Ep4Reg, Ep5Reg, Ep6Reg, Ep7Reg>{};
        static constexpr auto _endpoints = template_utils::type_list<Endpoints...>{};
    
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
            return GetRegisterNumber(template_utils::type_box<Endpoint>{});
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
            return _endpointRegs.template get<GetRegisterNumber(template_utils::type_box<Endpoint>{})>();
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    EndpointRegistersManager(template_utils::type_list<Endpoints...> endpoints) -> EndpointRegistersManager<Endpoints...>;

    /**
     * @brief Implements endpoint`s buffers management
     * 
     * @tparam Endpoints All endpoints
     */

    template<typename... Endpoints>
    class EndpointsManager
    {
        /// USB PMA base address
        static constexpr uint32_t PmaBufferBase = USB_PMAADDR;
        /// All endpoints sorted by number
        static constexpr auto _sortedUniqueEndpoints = template_utils::type_list<Endpoints...>{}.remove_duplicates().sort([](auto first, auto second){ return first.Number < second.Number; });
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
        static constexpr uint32_t BufferOffset = GetBufferOffset(template_utils::type_box<Endpoint>{});

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
        static const uint32_t BdtCellOffset = GetBdtCellOffset(template_utils::type_box<Endpoint>{});

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
                    template_utils::type_unbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // TxBuffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // TxCount
                    PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + PmaBufferSize(Endpoint::MaxPacketSize)), // RxBuffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //RxCount
            typename std::conditional_t<Endpoint::Type == EndpointType::BulkDoubleBuffered,
                BulkDoubleBufferedEndpoint<Endpoint,
                    template_utils::type_unbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer0
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2), // Buffer0Count
                    PmaBufferBase + PmaAlignMultiplier * (BufferOffset<Endpoint> + PmaBufferSize(Endpoint::MaxPacketSize)), // Buffer1
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 6)>, //Buffer1Count
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::In,
                InEndpoint<Endpoint,
                    template_utils::type_unbox<_registersManager.template GetEndpointReg<Endpoint>()>,
                    PmaBufferBase + PmaAlignMultiplier * BufferOffset<Endpoint>, // Buffer
                    PmaBufferBase + PmaAlignMultiplier * (BdtCellOffset<Endpoint> + 2)>, // BufferCount
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Out,
                OutEndpoint<Endpoint,
                    template_utils::type_unbox<_registersManager.template GetEndpointReg<Endpoint>()>,
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
                PmaHalfWordSet(BdtBase + PmaAlignMultiplier * GetBdtCellOffset(endpoint), GetBufferOffset(endpoint));
                PmaHalfWordSet(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 2), 0u);
            });
        }
        
        static void InitRxAddressFieldInDescriptor()
        {
            constexpr auto bidirectionalAndBulkDoubleBufferedEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint){
                return endpoint.Direction == EndpointDirection::Bidirectional || endpoint.Type == EndpointType::BulkDoubleBuffered;
            });

            bidirectionalAndBulkDoubleBufferedEndpoints.foreach([](auto endpoint){
                PmaHalfWordSet(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 4), GetBufferOffset(endpoint) + PmaBufferSize(endpoint.MaxPacketSize));
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
                PmaHalfWordSet(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 2), CalculateRxCountValue(endpoint));
            });
        }

        static void InitSecondRxCountFieldInDescriptor()
        {
            constexpr auto bidirectionalAndBulkDoubleBufferedEndpoints = _sortedUniqueEndpoints.filter([](auto endpoint){
                return endpoint.Direction == EndpointDirection::Bidirectional || endpoint.Type == EndpointType::BulkDoubleBuffered;
            });

            bidirectionalAndBulkDoubleBufferedEndpoints.foreach([](auto endpoint) {
                PmaHalfWordSet(BdtBase + PmaAlignMultiplier * (GetBdtCellOffset(endpoint) + 6), CalculateRxCountValue(endpoint));
            });
        }

        static consteval uint16_t CalculateRxCountValue(auto endpoint)
        {
            return endpoint.MaxPacketSize <= 62
                ? (endpoint.MaxPacketSize / 2) << 10
                : 0x8000 | ((endpoint.MaxPacketSize / 32 - 1) << 10);
        }
    };

    template<typename... Ep>
    EndpointsManager(template_utils::type_list<Ep...> endpoints) -> EndpointsManager<Ep...>;

    /**
     * @brief Endpoints initalizer
     * 
     * @tparam Endpoints All endpoints list
     */
    template<typename... Endpoints>
    using EndpointsInitializer = EndpointsManager<Endpoints...>;

}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINTS_MANAGER_H
