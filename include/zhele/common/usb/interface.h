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

#include <array>

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

        constexpr auto GetBytes() const
        {
            return std::array<uint8_t, 9> {
                Length,
                static_cast<uint8_t>(Type),
                Number,
                AlternateSetting,
                EndpointsCount,
                static_cast<uint8_t>(Class),
                SubClass,
                Protocol,
                StringIndex
            };
        }
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
        /**
         * @brief Returns nested interface descriptor total size
         * 
         * @returns Size in bytes
        */
        static consteval unsigned NestedDescriptorSize()
        {
            unsigned size = 0;

            Endpoints.foreach([&size](auto endpoint) {
                size += endpoint.GetDescriptor().size();
            });

            return size;
        }

    public:
        static const uint16_t Number = _Number;

        static constexpr auto Endpoints = Zhele::TemplateUtils::TypeList<_Endpoints...>{};
        static constexpr auto EndpointsCount = (0 + ... + (_Endpoints::Direction == EndpointDirection::Bidirectional ? 2 : 1));

        /**
         * @brief Reset interface
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            Endpoints.foreach([](auto endpoint) {
                endpoint.Reset();
            });
        }

        /**
         * @brief Build interface descriptor
         * 
         * @return Bytes of descriptor
         */
        static consteval auto GetDescriptor()
        {
            constexpr uint16_t size = sizeof(InterfaceDescriptor) + NestedDescriptorSize();
            std::array<uint8_t, size> result;

            constexpr auto head = InterfaceDescriptor {
                .Number = _Number,
                .AlternateSetting = _AlternateSetting,
                .EndpointsCount = EndpointsCount,
                .Class = _Class,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            }.GetBytes();
            auto dst = std::copy(head.begin(), head.end(), result.begin());

            Endpoints.foreach([&dst](auto endpoint) {
                auto nextEndpointDescriptor = Zhele::TemplateUtils::TypeUnbox<endpoint>::GetDescriptor();
                dst = std::copy(nextEndpointDescriptor.begin(), nextEndpointDescriptor.end(), dst);
            });

            return result;
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
     * @tparam Interfaces Interfaces typelist.
     */
    template<typename... Interfaces>
    class InterfaceHandlers
    {
        /**
         * @brief Builds indexes array for given interfaces
         * 
         * @returns std::array with indexes
        */
        static consteval auto BuildIndexesArray()
        {
            constexpr auto interfaces = Zhele::TemplateUtils::TypeList<Interfaces...>{};
            constexpr auto maxInterfaceNumber = interfaces.sort([](auto a, auto b) { return a.Number < b.Number; }).back().Number;
            std::array<int8_t, maxInterfaceNumber + 1> indexes {};

            for(auto& i : indexes) {
                i = -1;
            }

            interfaces.foreach([&indexes, i{0}](auto endpoint) mutable {
                indexes[endpoint.Number] = i++;
            });

            return indexes;
        }

        static constexpr InterfaceSetupRequestHandler _handlers[] = {Interfaces::SetupHandler...};
        static constexpr auto _handlersIndexes = BuildIndexesArray();
    public:
        constexpr InterfaceHandlers(auto interfaces) {}

        inline static void HandleSetupRequest(uint8_t number)
        {
            _handlers[_handlersIndexes[number]]();
        }
    };
    template<typename... Interfaces>
    InterfaceHandlers(Zhele::TemplateUtils::TypeList<Interfaces...> interfaces) -> InterfaceHandlers<Interfaces...>;
}
#endif // ZHELE_USB_INTERFACE_H