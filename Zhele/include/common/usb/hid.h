/**
 * @file
 * Implement USB-HID class
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_HID_H
#define ZHELE_USB_HID_H

#include "interface.h"

#include "../template_utils/type_list.h"

namespace Zhele::Usb
{
    template<uint8_t... _Data>
    struct HidReport
    {
        static constexpr uint8_t Data[sizeof...(_Data)] = {_Data...};
    };

    template<uint16_t _Version = 0x0200, typename... _Reports>
    struct HidDescriptor
    {
        uint8_t Length = 0x06 + 3 *  sizeof...(_Reports);
        uint8_t DescriptorType = 0x21;
        uint16_t Version = _Version;
        uint8_t CountryCode = 0x00;
        uint8_t ReportsCount = sizeof...(_Reports);

        static uint16_t FillReports(uint8_t* address)
        {
            ((*address++ = 0x22,
            *address++ = static_cast<uint8_t>(static_cast<uint16_t>(sizeof(_Reports::Data)) & 0xff),
            *address++ = static_cast<uint8_t>((static_cast<uint16_t>(sizeof(_Reports::Data)) >> 8) & 0xff)),...);

            return 3 * sizeof...(_Reports);
        }
    };

    template <uint8_t _Number, uint8_t _AlternateSetting, uint8_t _SubClass, uint8_t _Protocol, typename _Hid, typename... _Endpoints>
    class HidInterface : public Interface<_Number, _AlternateSetting, 0x03, _SubClass, _Protocol, _Endpoints...>
    {
        using Base = Interface<_Number, _AlternateSetting, 0x03, _SubClass, _Protocol, _Endpoints...>;
    public:
        using Endpoints = Base::Endpoints;

        static uint16_t FillDescriptor(InterfaceDescriptor* descriptor)
        {
            uint16_t totalLength = sizeof(InterfaceDescriptor);

            *descriptor = InterfaceDescriptor {
                .Number = _Number,
                .AlternateSetting = _AlternateSetting,
                .EndpointsCount = Base::EndpointsCount,
                .Class = 0x03,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            };
            _Hid* hidDescriptor = reinterpret_cast<_Hid*>(++descriptor);
            *hidDescriptor = _Hid {
            };
            uint8_t* reportsPart = reinterpret_cast<uint8_t*>(++hidDescriptor);
            uint16_t bytesWritten = _Hid::FillReports(reportsPart);

            totalLength += sizeof(_Hid) + bytesWritten;

            EndpointDescriptor* endpointsDescriptors = reinterpret_cast<EndpointDescriptor*>(&reportsPart[bytesWritten]);
            totalLength += (_Endpoints::FillDescriptor(endpointsDescriptors++) + ...);

            return totalLength;
        }
    private:
    };
}
#endif // ZHELE_USB_HID_H