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
    public:
        using Interfaces = Zhele::TemplateUtils::TypeList<_Interfaces...>;
        using Endpoints = Zhele::TemplateUtils::Append_t<typename _Interfaces::Endpoints...>;

        /**
         * @brief Resets configuration
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            (_Interfaces::Reset(), ...);
        }

        /**
         * @brief Fills descriptor
         * 
         * @param [out] descriptor Destination memory
         *
         * @returns Total bytes written
         */
        static uint16_t FillDescriptor(ConfigurationDescriptor* descriptor)
        {
            uint16_t totalLength = sizeof(ConfigurationDescriptor);

            *descriptor = ConfigurationDescriptor {
                .InterfacesCount = sizeof...(_Interfaces),
                .Number = _Number,
                .Attributes = {.RemoteWakeup = _RemoteWakeup, .SelfPowered = _SelfPowered},
                .MaxPower = _MaxPower
            };

            uint8_t* interfacesDescriptors = reinterpret_cast<uint8_t*>(descriptor);
            ((totalLength += _Interfaces::FillDescriptor(reinterpret_cast<InterfaceDescriptor*>(&interfacesDescriptors[totalLength]))), ...);

            descriptor->TotalLength = totalLength;

            return totalLength;
        }
    };
}
#endif // ZHELE_USB_CONFIGURATION_H