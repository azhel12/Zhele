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
        static LineCoding _lineCoding;
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
                .Class = DeviceAndInterfaceClass::Comm,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            };
        
            uint8_t* functionalDescriptors = reinterpret_cast<uint8_t*>(descriptor);

            ((totalLength += _Functionals::FillDescriptor(&functionalDescriptors[totalLength])), ...);

            EndpointDescriptor* endpointDescriptors = reinterpret_cast<EndpointDescriptor*>(&functionalDescriptors[totalLength]);
            totalLength += _Endpoint::FillDescriptor(endpointDescriptors);

            return totalLength;
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
                .Class = DeviceAndInterfaceClass::CdcData,
                .SubClass = _SubClass,
                .Protocol = _Protocol
            };

            uint8_t* endpointsDescriptors = reinterpret_cast<uint8_t*>(descriptor);
            ((totalLength += _Endpoints::FillDescriptor(reinterpret_cast<EndpointDescriptor*>(&endpointsDescriptors[totalLength]))), ...);

            return totalLength;
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
        static constexpr uint8_t _data[sizeof...(_Data)] = {_Data...};
    public:
        static uint8_t FillDescriptor(void* address)
        {
            uint8_t* destination = reinterpret_cast<uint8_t*>(address);
            
            destination[0] = 3 + sizeof...(_Data); // bFunctionLength
            destination[1] = _DescriptorType; // bDescriptorType
            destination[2] = _DescriptorSubtype; // bDescriptorSubtype

            memcpy(&destination[3], _data, sizeof...(_Data)); // Data

            return 3 + sizeof...(_Data);
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