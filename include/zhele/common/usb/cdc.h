/**
 * @file
 * Implement USB-CDC class
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_CDC_H
#define ZHELE_USB_CDC_H

#include "interface.h"

#include "../template_utils/type_list.h"

#include <string.h>

namespace Zhele::Usb
{
    /**
     * @brief CDC standart requests
     */
    enum class CdcRequest : uint8_t
    {
        SetLineCoding = 0x20, ///< Set line coding
        GetLineCoding = 0x21, ///< Get line coding
        SetControlLineState = 0x22, ///< Set control line state
    };

#pragma pack(push, 1)
    /**
     * @brief Line coding
     */
    struct LineCoding
    {
        uint32_t BaudRate; ///< Baud rate
        uint8_t CharFormat; ///< Char format
        uint8_t ParityType; ///< Parity 
        uint8_t DataBits; ///< Data bits
    };
#pragma pack(pop)

    /**
     * @brief Implements CDC communication interface
     * 
     * @tparam _Number Interface number
     * @tparam _AlternateSetting Interface alternate setting
     * @tparam _SubClass Interface subclass
     * @tparam _Protocol Interface protocol
     * @tparam _Ep0 Zero endpoint instance
     * @tparam _Endpoint Endpoint instance
     * @tparam _Functionals Functionals
     */
    template <uint8_t _Number, uint8_t _AlternateSetting, uint8_t _SubClass, uint8_t _Protocol, typename _Ep0, typename _Endpoint, typename... _Functionals>
    class CdcCommInterface : public Interface<_Number, _AlternateSetting, DeviceAndInterfaceClass::Comm, _SubClass, _Protocol, _Ep0, _Endpoint>
    {
        using Base = Interface<_Number, _AlternateSetting, DeviceAndInterfaceClass::Comm, _SubClass, _Protocol, _Ep0, _Endpoint>;
        static constexpr auto Functionals = Zhele::TemplateUtils::TypeList<_Functionals...>{};

        static LineCoding _lineCoding;

        /**
         * @brief Returns nested interface descriptor total size
         * 
         * @returns Size in bytes
        */
        static consteval unsigned NestedDescriptorSize()
        {
            unsigned size = 0;

            Base::Endpoints.foreach([&size](auto endpoint) {
                size += endpoint.GetDescriptor().size();
            });

            Functionals.foreach([&size](auto functional) {
                size += functional.GetDescriptor().size();
            });

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

            switch (static_cast<CdcRequest>(setup->Request))
            {
            case CdcRequest::SetLineCoding:
                if(setup->Length == 7)
                {
                    // Wait line coding
                    _Ep0::SetOutDataTransferCallback([]{
                        memcpy(&_lineCoding, reinterpret_cast<const void*>(_Ep0::RxBuffer), 7);
                        _Ep0::ResetOutDataTransferCallback();
                        _Ep0::SendZLP();
                    });
                    _Ep0::SetRxStatus(EndpointStatus::Valid);
                }
                break;
            case CdcRequest::GetLineCoding:
                _Ep0::SendData(&_lineCoding, sizeof(LineCoding));
                break;
            case CdcRequest::SetControlLineState:
                _Ep0::SendZLP();
                break;
            default:
                break;
            }
        }

        /**
         * @brief Build interface descriptor
         * 
         * @returns Bytes of interface descriptor
         */
        static consteval auto FillDescriptor()
        {
            constexpr unsigned size = sizeof(InterfaceDescriptor) + NestedDescriptorSize();
            std::array<uint8_t, size> result;
            
            constexpr auto head = InterfaceDescriptor {
                .Number = _Number,
                .AlternateSetting = _AlternateSetting,
                .EndpointsCount = Base::EndpointsCount,
                .Class = DeviceAndInterfaceClass::Comm,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            }.GetBytes();
            auto dst = std::copy(head.begin(), head.end(), result.begin());

            Functionals.foreach([&dst](auto functional){
                auto nextFunctionalDescriptor = Zhele::TemplateUtils::TypeUnbox<functional>::GetDescriptor();
                dst = std::copy(nextFunctionalDescriptor.begin(), nextFunctionalDescriptor.end(), dst);
            });
        
            Base::Endpoints.foreach([&dst](auto endpoint) {
                auto nextEndpointDescriptor = Zhele::TemplateUtils::TypeUnbox<endpoint>::GetDescriptor();
                dst = std::copy(nextEndpointDescriptor.begin(), nextEndpointDescriptor.end(), dst);
            });

            return result;
        }
    };
    template <uint8_t _Number, uint8_t _AlternateSetting, uint8_t _SubClass, uint8_t _Protocol, typename _Ep0, typename _Endpoint, typename... _Functionals>
    LineCoding CdcCommInterface<_Number, _AlternateSetting, _SubClass, _Protocol, _Ep0, _Endpoint, _Functionals...>::_lineCoding = {115200, 0, 0, 8};

    /**
     * @brief Implements CDC communication interface
     * 
     * @tparam _Number Interface number
     * @tparam _AlternateSetting Interface alternate setting
     * @tparam _SubClass Interface subclass
     * @tparam _Protocol Interface protocol
     * @tparam _Ep0 Zero endpoint instance
     * @tparam _Endpoint Endpoint

     */
    template <uint8_t _Number, uint8_t _AlternateSetting, uint8_t _SubClass, uint8_t _Protocol, typename _Ep0, typename... _Endpoints>
    class CdcDataInterface : public Interface<_Number, _AlternateSetting, DeviceAndInterfaceClass::CdcData, _SubClass, _Protocol, _Ep0, _Endpoints...>
    {
        using Base = Interface<_Number, _AlternateSetting, DeviceAndInterfaceClass::CdcData, _SubClass, _Protocol, _Ep0, _Endpoints...>;
    public:
        /**
         * @brief Interface setup request handler
         * 
         * @par Returns
         *  Nothing
         */
        static void SetupHandler()
        {
        }
    };

    /**
     * @brief Implements functional descriptor wrapper
     * 
     * @tparam _DescriptorType Descriptor type
     * @tparam _DescriptorSubtype Descriptor subtype
     * @tparam _Data Some data
     */
    template<uint8_t _DescriptorType, uint8_t _DescriptorSubtype, uint8_t... _Data>
    class FunctionalDescriptor
    {
        static constexpr std::array<uint8_t, sizeof...(_Data)> _data = {_Data...};
    public:
        /**
         * @brief Returns functional descriptor
         * 
         * @return Bytes of descriptor
        */
        static consteval auto GetDescriptor()
        {
            std::array<uint8_t, 3 + sizeof...(_Data)> result {
                3 + sizeof...(_Data),
                _DescriptorType,
                _DescriptorSubtype
            };
            auto dst = result.begin();
            std::advance(dst, 3);
            
            std::copy(_data.begin(), _data.end(), dst);

            return result;
        }
    };

    /**
     * @brief CDC interface subclass
     */
    enum class CdcInterfaceSubClass : uint8_t
    {
        DirectLine = 0x01, ///< Direct line control model
        Abstract = 0x02, ///< Abstract Control Model
        Telephone = 0x03, ///< Telephone Control Model
        MultiChannel = 0x04, ///< Multi-Channel Control Model
        CAPI = 0x05, ///< CAPI Control Model
        Ethernet = 0x06, ///< Ethernet Networking Control Model
        Atm = 0x07, ///< ATM Networking Control Model
        Wireless = 0x08, ///< Wireless Handset Control Model
        DeviceManagement = 0x09, ///< Device Management
        MobileDirectLine = 0x0a, ///< Mobile Direct Line Model
        Obex = 0x0b, ///< OBEX
    };
    
    /**
     * @brief Implements interface functional descriptor wrapper (CS_INTERFACE)
     * 
     * @tparam _DescriptorSubtype Descriptor subtype
     * @tparam _Data Some data
     */
    template<uint8_t _DescriptorSubtype, uint8_t... _Data>
    using InterfaceFunctionalDescriptor = FunctionalDescriptor<0x24, _DescriptorSubtype, _Data...>;
    
    /**
     * @brief Implements endpoint functional descriptor wrapper (CS_ENDPOINT)
     * 
     * @tparam _DescriptorSubtype Descriptor subtype
     * @tparam _Data Some data
     */
    template<uint8_t _DescriptorSubtype, uint8_t... _Data>
    using EndpointFunctionalDescriptor = FunctionalDescriptor<0x25, _DescriptorSubtype, _Data...>;

    using HeaderFunctional = InterfaceFunctionalDescriptor<0x00, 0x10, 0x01>;
    using CallManagementFunctional = InterfaceFunctionalDescriptor<static_cast<uint8_t>(CdcInterfaceSubClass::DirectLine), 0x00, 0x01>;
    using AcmFunctional = InterfaceFunctionalDescriptor<static_cast<uint8_t>(CdcInterfaceSubClass::Abstract), 0x02>;
    using UnionFunctional = InterfaceFunctionalDescriptor<static_cast<uint8_t>(CdcInterfaceSubClass::Ethernet), 0x00, 0x01>;

    /**
     * @brief Default CDC communication interface (with functionals: header, call management, acm and union)
     * 
     * @tparam _Number Interface number
     * @tparam _Ep0 Zero endpoint
     * @tparam _Endpoint Communication endpoint
     */
    template<uint8_t _Number, typename _Ep0, typename _Endpoint>
    using DefaultCdcCommInterface = CdcCommInterface<_Number, 0, 0x02, 0x01, _Ep0, _Endpoint, HeaderFunctional, CallManagementFunctional, AcmFunctional, UnionFunctional>;
}
#endif // ZHELE_USB_CDC_H