/**
 * @file
 * Implement USB-CDC class
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
    public:
        using Endpoints = Base::Endpoints;

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
                uint8_t temp[HidInterface::ReportsSize()];
                uint16_t size = HidInterface::FillReports(temp);
                _Ep0::SendData(temp, setup->Length < size ? setup->Length : size);
            }
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
                .EndpointsCount = Base::EndpointsCount,
                .Class = InterfaceClass::Hid,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            };
            _HidImpl* hidDescriptor = reinterpret_cast<_HidImpl*>(++descriptor);
            *hidDescriptor = _HidImpl {
            };
            
            uint8_t* reportsPart = reinterpret_cast<uint8_t*>(++hidDescriptor);
            uint16_t bytesWritten = _HidImpl::FillReportsDescriptors(reportsPart);

            totalLength += sizeof(_HidImpl) + bytesWritten;

            EndpointDescriptor* endpointsDescriptors = reinterpret_cast<EndpointDescriptor*>(&reportsPart[bytesWritten]);
            totalLength += (0 + ... + _Endpoints::FillDescriptor(endpointsDescriptors++));

            return totalLength;
        }

        /**
         * @brief Returns reports size
         * 
         * @returns Reports size
         */
        static constexpr uint16_t ReportsSize()
        {
            return _HidImpl::ReportsSize();
        }

        /**
         * @brief Fills reports
         * 
         * @param [out] descriptor Destination memory
         * 
         * @returns Total bytes written
         */
        static uint16_t FillReports(uint8_t* destination)
        {
            return _HidImpl::FillReports(destination);
        }
    private:
    };
}
#endif // ZHELE_USB_HID_H