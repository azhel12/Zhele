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
    struct ConfigurationAttributes
    {
        uint8_t Reserved : 5;
        bool RemoteWakeup : 1 = false;
        bool SelfPowered : 1 = false;
        uint8_t Reserved2 : 1;
    };

    struct ConfigurationDescriptor
    {
        static const uint8_t Length = 9;
        static const DescriptorType Type = DescriptorType::Configuration;
        uint16_t TotalLength;
        uint8_t InterfacesCount;
        uint8_t Number;
        uint8_t StringIndex = 0;
        ConfigurationAttributes Attributes;
        uint8_t MaxPower;
    };

    /**
     * @brief Implements configuration
     * 
     * @tparam _Number Configuration value
     */
    template <uint8_t _Number, uint8_t _MaxPower, bool _RemoteWakeup = false, bool _SelfPowered = false, typename... _Interfaces>
    class Configuration
    {
    public:
        using Endpoints = Zhele::TemplateUtils::Append_t<typename _Interfaces::Endpoints...>;

        static void Reset()
        {
            (_Interfaces::Reset(), ...);
        }

        static uint16_t FillDescriptor(ConfigurationDescriptor* descriptor)
        {
            uint16_t totalLength = sizeof(ConfigurationDescriptor);

            *descriptor = ConfigurationDescriptor {
                .InterfacesCount = sizeof...(_Interfaces),
                .Number = _Number,
                .Attributes = {.RemoteWakeup = _RemoteWakeup, .SelfPowered = _SelfPowered},
                .MaxPower = _MaxPower
            };

            InterfaceDescriptor* configurationsDescriptors = reinterpret_cast<InterfaceDescriptor*>(reinterpret_cast<uint8_t*>(descriptor) + sizeof(ConfigurationDescriptor));
            totalLength += (_Interfaces::FillDescriptor(configurationsDescriptors++) + ...);

            descriptor->TotalLength = totalLength;

            return totalLength;
        }
    private:
    };
}
#endif // ZHELE_USB_CONFIGURATION_H