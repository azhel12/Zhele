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

#include <string.h>

namespace Zhele::Usb
{
    /**
     * @brief Implements HID report.
     * 
     * @tparam _Data Raw report bytes.
     */
    template<uint8_t... _Data>
    struct HidReport
    {
        static constexpr std::array<uint8_t, sizeof...(_Data)> Data = {_Data...};
    };

    /**
     * @brief Implements HID descriptor.
     * 
     * @tparam _Version Version.
     * @tparam _Reports All reports.
     */
    template<uint16_t _Version = 0x0200, typename... _Reports>
    class HidImpl
    {
        static constexpr auto Reports = Zhele::TemplateUtils::TypeList<_Reports...>{};

        /**
         * @brief Total reports size
         * 
         * @returns Reports size
         */
        static consteval unsigned GetReportsSize()
        {
            unsigned size = 0;

            Reports.foreach([&size](auto report) {
                size += report.Data.size();
            });

            return size;
        }

    public:

        /**
         * @brief Build reports descriptors
         * 
         * @returns Report descriptor bytes
         */
        static consteval auto GetDescriptor()
        {
            constexpr unsigned size = 0x06 + 3 * sizeof...(_Reports);

            std::array<uint8_t, size> result = {
                size,                                       // Lenght
                0x21,                                       // Descriptor type
                _Version & 0xff, (_Version >> 8) & 0xff,    // Version
                0x00,                                       // County code
                sizeof...(_Reports)                         // Reports count
            };

            auto dst = result.begin();
            std::advance(dst, 0x06);

            Reports.foreach([&dst](auto report) {
                *(dst++) = 0x22;
                *(dst++) = report.Data.size() & 0xff;
                *(dst++) = (report.Data.size() >> 8) & 0xff;
            });

            return result;
        }

        /**
         * @brief Fill HID reports
         * 
         * @param [out] address Destination memory
         * 
         * @returns Total bytes writter
         */
        static consteval auto GetReports()
        {
            constexpr auto size = GetReportsSize();
            
            std::array<uint8_t, size> result;
            auto dst = result.begin();

            Reports.foreach([&dst](auto report) {
                dst = std::copy(report.Data.begin(), report.Data.end(), dst);
            });

            return result;
        }
    };

    /**
     * @brief Implements HID interface
     * 
     * @tparam _Number Interface number
     * @tparam _AlternateSetting Interface alternate setting
     * @tparam _Class Interface class
     * @tparam _SubClass Interface subclass
     * @tparam _Protocol Interface protocol
     * @tparam _HidImpl HidImpl instance
     * @tparam _Ep0 Zero endpoint instance
     * @tparam _Endpoints Endpoints 
     * 
     */
    template <uint8_t _Number, uint8_t _AlternateSetting, uint8_t _SubClass, uint8_t _Protocol, typename _HidImpl, typename _Ep0, typename... _Endpoints>
    class HidInterface : public Interface<_Number, _AlternateSetting, InterfaceClass::Hid, _SubClass, _Protocol, _Ep0, _Endpoints...>
    {
        using Base = Interface<_Number, _AlternateSetting, InterfaceClass::Hid, _SubClass, _Protocol, _Ep0, _Endpoints...>;
        

        static consteval unsigned NestedDescriptorSize()
        {
            unsigned size = 0;

            Base::Endpoints.foreach([&size](auto endpoint) {
                size += endpoint.GetDescriptor().size();
            });

            size += _HidImpl::GetDescriptor().size();

            return size;
        }
    public:

        /**
         * @brief Interface setup request handler
         * 
         * @par Returns
         *  Nothing
         */
        static void SetupHandler()
        {
            SetupPacket* setup = reinterpret_cast<SetupPacket*>(_Ep0::RxBuffer);

            if (setup->Request == StandartRequestCode::GetDescriptor
                && static_cast<GetDescriptorParameter>(setup->Value) == GetDescriptorParameter::HidReportDescriptor)
            {
                constexpr auto descriptor = _HidImpl::GetReports();
                _Ep0::SendData(descriptor.data(), setup->Length < descriptor.size() ? setup->Length : descriptor.size());
            }
        }

        /**
         * @brief Build HID interface descriptor
         * 
         * @returns Bytes of interface descriptor
         */
        static consteval auto GetDescriptor()
        {
            constexpr unsigned size = sizeof(InterfaceDescriptor) + NestedDescriptorSize();
            std::array<uint8_t, size> result;
            auto dst = result.begin();

            constexpr auto interfacefDesc = InterfaceDescriptor {
                .Number = _Number,
                .AlternateSetting = _AlternateSetting,
                .EndpointsCount = Base::EndpointsCount,
                .Class = InterfaceClass::Hid,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            }.GetBytes();
            dst = std::copy(interfacefDesc.begin(), interfacefDesc.end(), dst);

            constexpr auto reportDescriptors = _HidImpl::GetDescriptor();
            dst = std::copy(reportDescriptors.begin(), reportDescriptors.end(), dst);

            Base::Endpoints.foreach([&dst](auto endpoint){
                auto nextEndpointDescriptor = endpoint.GetDescriptor();
                dst = std::copy(nextEndpointDescriptor.begin(), nextEndpointDescriptor.end(), dst);
            });

            return result;
        }
    };
}
#endif // ZHELE_USB_HID_H