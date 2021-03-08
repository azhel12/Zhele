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

#include "endpoint.h"

namespace Zhele::Usb
{
    struct InterfaceDescriptor
    {
        static const uint8_t Length = 9;
        static const DescriptorType Type = DescriptorType::Interface;
        uint8_t Number;
        uint8_t AlternateSetting = 0;
        uint8_t EndpointsCount;
        uint8_t Class = 0;
        uint8_t SubClass = 0;
        uint8_t Protocol = 0;
        uint8_t StringIndex = 0;
    };
    /**
     * @brief Implements interface
     * 
     * @tparam _Number Interface number
     * 
     */
    template <uint8_t _Number, uint8_t _AlternateSetting = 0, uint8_t _Class = 0, uint8_t _SubClass = 0, uint8_t _Protocol = 0, typename... _Endpoints>
    class Interface
    {
    public:
        using Endpoints = Zhele::TemplateUtils::TypeList<_Endpoints...>;
        static const uint8_t EndpointsCount = ((_Endpoints::Direction == EndpointDirection::Bidirectional ? 2 : 1) + ...);

        static void Reset()
        {
            (_Endpoints::Reset(), ...);
        }

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
            
            EndpointDescriptor* endpointsDescriptors = reinterpret_cast<EndpointDescriptor*>(reinterpret_cast<uint8_t*>(descriptor) + sizeof(InterfaceDescriptor));
            totalLength += (_Endpoints::FillDescriptor(endpointsDescriptors++) + ...);

            return totalLength;
        }
    private:
    };
}
#endif // ZHELE_USB_INTERFACE_H