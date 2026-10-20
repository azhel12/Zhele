/**
 * @file
 * Implement endpoint`s registers and FIFO allocation for Synopsys OTG core
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINTS_MANAGER_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINTS_MANAGER_H

#include "endpoint.h"
#include "../../ioreg.h"

#include <zhele/common/template_utils/type_list.h>

#include <type_traits>
#include <stdint.h>

namespace Zhele::Usb
{
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

        static constexpr auto _endpointInRegs = Zhele::template_utils::type_list<InEp0Reg, InEp1Reg, InEp2Reg, InEp3Reg>{};
        static constexpr auto _endpointOutRegs = Zhele::template_utils::type_list<OutEp0Reg, OutEp1Reg, OutEp2Reg, OutEp3Reg>{};

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
                return template_utils::type_box<void>{};
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
                return template_utils::type_box<void>{};
            else
                return _endpointOutRegs.template get<Number>();
        }
    };
    // deduction guide for OffsetCalculator
    template<typename... Endpoints>
    EndpointRegistersManager(template_utils::type_list<Endpoints...> endpoints) -> EndpointRegistersManager<Endpoints...>;

    template<typename... Endpoints>
    class EndpointsManager
    {
        IO_STRUCT_WRAPPER(USB_OTG_FS, OtgFsGlobal, USB_OTG_GlobalTypeDef);
        /// All endpoints sorted by number
        static constexpr auto _sortedUniqueEndpoints = template_utils::type_list<Endpoints...>{}.remove_duplicates().sort([](auto first, auto second){ return first.Number < second.Number; });
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
        /// FIFO base address
        static constexpr uint32_t _fifoBaseAddress = USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE;
        /// FIFO size for one endpoint
        static constexpr uint32_t _epFifoSize = USB_OTG_FIFO_SIZE;

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
                    template_utils::type_unbox<_registersManager.template GetEndpointInReg<Endpoint::Number>()>, // IN register
                    template_utils::type_unbox<_registersManager.template GetEndpointOutReg<Endpoint::Number>()>, // OUT register
                    static_cast<uint8_t>(_sortedUniqueInEndpoints.template search<Endpoint>()), // Fifo number
                    _fifoBaseAddress + Endpoint::Number * _epFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::Out,
                OutEndpoint<
                    Endpoint,
                    template_utils::type_unbox<_registersManager.template GetEndpointOutReg<Endpoint::Number>()>, // OUT register
                    _fifoBaseAddress + Endpoint::Number * _epFifoSize>, // Fifo address = Fifo base + i * USB_OTG_FIFO_SIZE
            typename std::conditional_t<Endpoint::Direction == EndpointDirection::In,
                InEndpoint<
                    Endpoint,
                    template_utils::type_unbox<_registersManager.template GetEndpointInReg<Endpoint::Number>()>, // IN register
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
         * @brief Init TX fifo for endpoint
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
         * but minimum value IN endpoint TxFIFO depth is 16 (in terms of 32-bit words)
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
         * @brief Calculate sum of endpoints FIFO size
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
    template<typename... Ep>
    EndpointsManager(template_utils::type_list<Ep...> endpoints) -> EndpointsManager<Ep...>;

    /**
     * @brief Endpoints initalizer
     * 
     * @tparam Endpoints All endpoints list
     */
    template<typename... Endpoints>
    using EndpointsInitializer = EndpointsManager<Endpoints...>;

    /**
     * @brief Endpoint RX FIFO not empty handler
     */
    using EpRxFifoNotEmptyHandler = std::add_pointer_t<void(uint16_t size)>;

    /**
     * @brief Implements endpoint`s handlers management
     * 
     * @tparam Endpoints Unique sorted endpoints
     * @tparam Indexes Handlers indexes
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
            constexpr auto outEndpoints = template_utils::type_list<OutEndpoints...>{};

            constexpr auto maxEndpointNumber = outEndpoints.sort([](auto a, auto b) { return template_utils::type_unbox<a>::Number < template_utils::type_unbox<b>::Number; }).back().Number;
            std::array<int8_t, maxEndpointNumber + 1> indexes {};

            for(auto& i : indexes) {
                i = -1;
            }

            outEndpoints.foreach([&indexes, i{0}](auto endpoint) mutable {
                indexes[template_utils::type_unbox<endpoint>::Number] = i++;
            });

            return indexes;
        }

        /**
         * @todo Pack rx fifo handlers via add one In endpoints as template parameter
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
    EndpointFifoNotEmptyHandlers(template_utils::type_list<Ep...> endpoints) -> EndpointFifoNotEmptyHandlers<Ep...>;
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINTS_MANAGER_H
