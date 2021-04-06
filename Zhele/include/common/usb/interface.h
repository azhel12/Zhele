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
    #pragma pack(push, 1)
    struct InterfaceDescriptor
    {
        uint8_t Length = 9;
        DescriptorType Type = DescriptorType::Interface;
        uint8_t Number;
        uint8_t AlternateSetting = 0;
        uint8_t EndpointsCount;
        InterfaceClass Class = DeviceClass::InterfaceSpecified;
        uint8_t SubClass = 0;
        uint8_t Protocol = 0;
        uint8_t StringIndex = 0;
    };
    #pragma pack(pop)
    /**
     * @brief Implements interface
     * 
     * @tparam _Number Interface number
     * 
     */
    template <uint8_t _Number, uint8_t _AlternateSetting = 0, InterfaceClass _Class = InterfaceClass::InterfaceSpecified, uint8_t _SubClass = 0, uint8_t _Protocol = 0, typename... _Endpoints>
    class Interface
    {
    public:
        using Endpoints = Zhele::TemplateUtils::TypeList<_Endpoints...>;
        static const uint8_t EndpointsCount = (0 + ... + (_Endpoints::Direction == EndpointDirection::Bidirectional ? 2 : 1));

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
            
            EndpointDescriptor* endpointsDescriptors = reinterpret_cast<EndpointDescriptor*>(++descriptor);
            totalLength += (0 + ... + _Endpoints::FillDescriptor(endpointsDescriptors++));

            return totalLength;
        }
    private:
    };
}
#endif // ZHELE_USB_INTERFACE_H