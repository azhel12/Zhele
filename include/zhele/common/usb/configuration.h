/**
 * @file
 * Implement USB configuration
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_CONFIGURATION_H
#define ZHELE_USB_CONFIGURATION_H

#include "interface.h"

namespace Zhele::Usb
{
    /**
     * @brief Configuration attributes
     */
    struct ConfigurationAttributes
    {
        uint8_t Reserved : 5; ///< Reserved
        bool RemoteWakeup : 1 = false; ///< Remote wakeup support
        bool SelfPowered : 1 = false; ///< Self-powered
        uint8_t Reserved2 : 1; ///< Reserved
    };

#pragma pack(push, 1)
    /**
     * @brief Configuration descriptor
     */
    struct ConfigurationDescriptor
    {
        uint8_t Length = 9; ///< Length (always 9)
        DescriptorType Type = DescriptorType::Configuration; ///< Descriptor type (always 0x02)
        uint16_t TotalLength; ///< Total descriptor length (included all interfaces and endpoints descriptors)
        uint8_t InterfacesCount; ///< Interfaces count
        uint8_t Number; ///< Configuration number
        uint8_t StringIndex = 0; ///< Configuration string index
        ConfigurationAttributes Attributes; ///< Configuration attributes
        uint8_t MaxPower; ///< Max power (in 2mA units)

        /**
         * @brief Returns descriptor bytes
         * 
         * @returns Bytes of descriptor
        */
        constexpr auto GetBytes()
        {
            return std::array<uint8_t, 9> {
                Length,
                static_cast<uint8_t>(Type),
                static_cast<uint8_t>(TotalLength & 0xff), static_cast<uint8_t>((TotalLength >> 8) & 0xff),
                InterfacesCount,
                Number,
                StringIndex,
                static_cast<uint8_t>(((Attributes.RemoteWakeup ? 1 : 0) << 5) | ((Attributes.SelfPowered ? 1 : 0) << 6)),
                MaxPower
            };
        }
    };
#pragma pack(pop)

    /**
     * @brief Implements configuration
     * 
     * @tparam _Number Configuration number
     * @tparam _MaxPower Max power (in 2mA units)
     * @tparam _RemoteWakeup Remote wakeup feature
     * @tparam _SelfPowered Self-powered feature
     * @tparam _Interfaces Interfaces
     */
    template <uint8_t _Number, uint8_t _MaxPower, bool _RemoteWakeup = false, bool _SelfPowered = false, typename... _Interfaces>
    class Configuration
    {
        /**
         * @brief Returns nested interface descriptor total size
         * 
         * @returns Size in bytes
        */
        static consteval unsigned NestedDescriptorSize()
        {
            unsigned size = 0;

            Interfaces.foreach([&size](auto interface) {
                size += interface.GetDescriptor().size();
            });

            return size;
        }
    public:
        static constexpr auto Interfaces = Zhele::TemplateUtils::TypeList<_Interfaces...>{};
        static constexpr auto Endpoints = (Zhele::TemplateUtils::TypeList<>{} + ... + _Interfaces::Endpoints);

        /**
         * @brief Resets configuration
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            Interfaces.foreach([](auto interface) {
                interface.Reset();
            });
        }

        /**
         * @brief Build descriptor
         * 
         * @returns Bytes of descriptor
         */
        static consteval auto GetDescriptor()
        {
            constexpr uint16_t size = sizeof(ConfigurationDescriptor) + NestedDescriptorSize();
            std::array<uint8_t, size> result;

            constexpr auto head = ConfigurationDescriptor {
                .TotalLength = size,
                .InterfacesCount = Interfaces.size(),
                .Number = _Number,
                .Attributes = {.RemoteWakeup = _RemoteWakeup, .SelfPowered = _SelfPowered},
                .MaxPower = _MaxPower
            }.GetBytes();

            auto dstEnd = std::copy(head.begin(), head.end(), result.begin());

            Interfaces.foreach([&result, &dstEnd](auto interface) {
                auto nextInterfaceDescriptor = Zhele::TemplateUtils::TypeUnbox<interface>::GetDescriptor();
                dstEnd = std::copy(nextInterfaceDescriptor.begin(), nextInterfaceDescriptor.end(), dstEnd);
            });

            return result;
        }
    };
}
#endif // ZHELE_USB_CONFIGURATION_H