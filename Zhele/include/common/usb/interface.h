/**
 * @file
 * Implement USB interface
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_INTERFACE_H
#define ZHELE_USB_INTERFACE_H

#include "common.h"
#include "endpoint.h"

#include "../template_utils/type_list.h"
#include "../template_utils/static_array.h"

namespace Zhele::Usb
{
    /**
     * @brief Interface descriptor
     */
#pragma pack(push, 1)
    struct InterfaceDescriptor
    {
        uint8_t Length = 9; ///< Length (always 9)
        DescriptorType Type = DescriptorType::Interface;///< Descriptor type (always 0x04)
        uint8_t Number; ///< Interface number
        uint8_t AlternateSetting = 0; ///< Interface alternate setting
        uint8_t EndpointsCount; ///< Endpoints count
        DeviceAndInterfaceClass Class = DeviceAndInterfaceClass::InterfaceSpecified; ///< Interface class
        uint8_t SubClass = 0; ///< Interface subclass
        uint8_t Protocol = 0; ///< Interface protocol
        uint8_t StringIndex = 0; ///< Interface string ID
    };
#pragma pack(pop)

    /**
     * @brief Implements interface
     * 
     * @tparam _Number Interface number
     * @tparam _AlternateSetting Interface alternate setting
     * @tparam _Class Interface class
     * @tparam _SubClass Interface subclass
     * @tparam _Protocol Interface protocol
     * @tparam _Ep0 Zero endpoint instance
     * @tparam _Endpoints Endpoints 
     * 
     */
    template <uint8_t _Number, uint8_t _AlternateSetting, DeviceAndInterfaceClass _Class, uint8_t _SubClass, uint8_t _Protocol, typename _Ep0, typename... _Endpoints>
    class Interface
    {
    public:
        static const uint16_t Number = _Number;

        using Endpoints = Zhele::TemplateUtils::TypeList<_Endpoints...>;
        static const uint8_t EndpointsCount = (0 + ... + (_Endpoints::Direction == EndpointDirection::Bidirectional ? 2 : 1));

        /**
         * @brief Reset interface
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            (_Endpoints::Reset(), ...);
        }

        /**
         * @brief Fills descriptor
         * 
         * @param [out] descriptor Destination memory
         * 
         * @par Returns
         *  Nothing
         */
        static uint16_t FillDescriptor(InterfaceDescriptor* descriptor)
        {
            uint16_t totalLength = sizeof(InterfaceDescriptor);

            *descriptor = InterfaceDescriptor {
                .Number = _Number,
                .AlternateSetting = _AlternateSetting,
                .EndpointsCount = EndpointsCount,
                .Class = _Class,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            };
            
            uint8_t* endpointsDescriptors = reinterpret_cast<uint8_t*>(descriptor);
            ((totalLength += _Endpoints::FillDescriptor(reinterpret_cast<EndpointDescriptor*>(&endpointsDescriptors[totalLength]))), ...);

            return totalLength;
        }

        /**
         * @brief Setup request handler
         * 
         * @par Returns
         *  Nothing
         */
        static void SetupHandler();
    };

    /**
     * @brief Interface transfer complete callback.
     */
    using InterfaceSetupRequestHandler = std::add_pointer_t<void()>;
    /**
     * @brief Implements Interface`s handlers management.
     * 
     * @tparam Interfaces Unique sorted Interfaces.
     * @tparam Indexes Handlers indexes.
     */
    
    template<typename...>
    class InterfaceHandlersBase;
    template<typename... Interfaces, int8_t... Indexes>
    class InterfaceHandlersBase<Zhele::TemplateUtils::TypeList<Interfaces...>, Zhele::TemplateUtils::Int8_tArray<Indexes...>>
    {
    public:
        static constexpr InterfaceSetupRequestHandler _handlers[] = {Interfaces::SetupHandler...};
        static constexpr int8_t _handlersIndexes[] = {Indexes...};
    public:
        inline static void HandleSetupRequest(uint8_t number)
        {
            _handlers[_handlersIndexes[number]]();
        }
    };

    /**
     * @brief Sort interfaces by number and direction
     */
    template <typename T, typename U>
    struct InterfaceNumberComparator : std::conditional_t<U::Number < T::Number, std::true_type, std::false_type>
    {};

    template<typename Interfaces>
    using InterfacesSortedByNumber = typename Zhele::TemplateUtils::TypeListSort<InterfaceNumberComparator, Interfaces>::type;

    /**
     * @brief Unique interfaces sorted by numbers.
     */
    template<typename Interfaces>
    using SortedUniqueInterfaces = InterfacesSortedByNumber<typename Zhele::TemplateUtils::Unique<Interfaces>::type>;

    /**
     * @brief Predicate for search interface by number
     * 
     * @tparam Number Interface number
     */
    template<uint8_t Number>
    class IsInterfaceWithNumber
    {
    public:
        template<typename Interface>
        class type
        {
        public:
            static const bool value = Interface::Number == Number;
        };
    };

    /**
     * @brief Implements interface`s handlers indexes management.
     * 
     * @tparam Index Handler index.
     * @tparam Interfaces Unique sorted interfaces.
     */
    template<int8_t Index, typename Interfaces>
    class InterfaceHandlersIndexes
    {
        using Predicate = IsInterfaceWithNumber<Index>;
        static const int8_t InterfaceIndex = Zhele::TemplateUtils::Search<Predicate::template type, Interfaces>::value;
    public:
        using type = typename Zhele::TemplateUtils::Int8_tArray_InsertBack<typename InterfaceHandlersIndexes<Index - 1, Interfaces>::type, InterfaceIndex>::type;
    };
    template<typename Interfaces>
    class InterfaceHandlersIndexes<-1, Interfaces>
    {
    public:
        using type = Zhele::TemplateUtils::Int8_tArray<>;
    };

    template<typename Interfaces>
    const int8_t MaxInterfaceNumber = Zhele::TemplateUtils::GetType<Zhele::TemplateUtils::Length<SortedUniqueInterfaces<Interfaces>>::value - 1, SortedUniqueInterfaces<Interfaces>>::type::Number;

    /**
     * @brief Interface`s handlers.
     */
    template<typename Interfaces>
    using InterfaceHandlers = InterfaceHandlersBase<SortedUniqueInterfaces<Interfaces>,
        typename InterfaceHandlersIndexes<MaxInterfaceNumber<Interfaces> + 1, SortedUniqueInterfaces<Interfaces>>::type>;
}
#endif // ZHELE_USB_INTERFACE_H