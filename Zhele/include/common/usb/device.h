/**
 * @file
 * Implement USB device
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_DEVICE_H
#define ZHELE_USB_DEVICE_H

#include "configuration.h"
#include "endpoints_manager.h"
#include "hid.h"
#include "cdc.h"

#include "../ioreg.h"
#include "../../common/template_utils/fixed_string.h"

#include <type_traits>

namespace Zhele::Usb
{
    /**
     * @brief Device descriptor.
     */
#pragma pack(push, 1)
    struct DeviceDescriptor
    {
        uint8_t Length = 18; ///< Length (always 18)
        DescriptorType Type = DescriptorType::Device; ///< Desciptor type (always 0x01)
        uint16_t UsbVersion; ///< Usb version
        DeviceAndInterfaceClass Class; ///< Device class
        uint8_t SubClass; ///< Device subclass
        uint8_t Protocol; ///< Device protocol
        uint8_t MaxPacketSize; ///< Ep0 maximum packet size
        uint16_t VendorId; ///< Vendor ID (VID)
        uint16_t ProductId; ///< Product ID (PID)
        uint16_t DeviceReleaseNumber; ///< Device release number
        uint8_t ManufacturerStringIndex = 0; ///< Manufacturer string ID
        uint8_t ProductStringIndex = 0; ///< Product string ID
        uint8_t SerialNumberStringIndex = 0; ///< Serial number string ID
        uint8_t ConfigurationsCount; ///< Configurations count
    };
#pragma pack(pop)
    
        /**
     * @brief String descriptor
     */
#pragma pack(push, 1)    
    struct LangIdDescriptor
    {
        uint8_t Length = sizeof(LangIdDescriptor);
        uint8_t DescriptorType = static_cast<uint8_t>(DescriptorType::String);
        uint16_t USLang = 0x0409;
    };
#pragma pack(pop)

    /**
     * @brief Implements USB device.
     * 
     * @tparam _Regs USB regs wrapper
     * @tparam _IRQNumber Irq number
     * @tparam _ClockCtrl Clock control
     * @tparam _UsbVersion Usb version
     * @tparam _Class Class
     * @tparam _SubClass Subclass
     * @tparam _Protocol Protocol
     * @tparam _VendorId Vendor ID
     * @tparam _ProductId Product ID
     * @tparam _DeviceReleaseNumber Device release number
     * @tparam _Ep0 Endpoint 0 base
     * @tparam _Configurations Device`s configurations
     */
    template<
        typename _Regs,
        IRQn_Type _IRQNumber,
        typename _ClockCtrl, 
        uint16_t _UsbVersion,
        DeviceAndInterfaceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        auto _Manufacturer,
        auto _Product,
        auto _Serial,
        typename _Ep0,
        typename... _Configurations>
    class DeviceBase : public _Ep0
    {
        using This = DeviceBase<_Regs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;

        using Configurations = TypeList<_Configurations...>;
        using Interfaces = Append_t<typename _Configurations::Interfaces...>; 
        using Endpoints = Append_t<typename _Configurations::Endpoints...>;

        using EpBufferManager = EndpointsManager<Append_t<_Ep0, Endpoints>>;
        // Replace Ep0 with this for correct handler register.
        using EpHandlers = EndpointHandlers<Append_t<This, Endpoints>>;
        using IfHandlers = InterfaceHandlers<Interfaces>;

        static uint8_t _tempAddressStorage;
        static volatile bool _isDeviceConfigured;
    public:
        /**
         * @brief Select clock source
         * 
         * @tparam T Clock source type
         * 
         * @param [in] clockSource Clock source
         * 
         * @par Returns
         *  Nothing
         */
        template<typename T>
        static void SelectClockSource(T clockSource);

        /**
         * @brief Enables device
         * 
         * @par Returns
         *  Nothing
         */
        static void Enable()
        {
            _ClockCtrl::Enable();
            EpBufferManager::Init();
            
            _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
            _Regs()->ISTR = 0;
            _Regs()->BTABLE = 0;
        #if defined (USB_BCDR_DPPU)
            _Regs()->BCDR |= USB_BCDR_DPPU;
        #endif

            NVIC_EnableIRQ(_IRQNumber);
        }

        /**
         * @brief Allows check that device configure complete
         * 
         * @retval true Device is configured
         * @retval false Device is not configured yet
         */
        static bool IsDeviceConfigured()
        {
            return _isDeviceConfigured;
        }

        /**
         * @brief Fills descriptor
         * 
         * @param [out] descriptor Destination memory
         * 
         * @par Returns
         *  Nothing
         */
        static void FillDescriptor(DeviceDescriptor* descriptor)
        {
           *descriptor = DeviceDescriptor {
                .UsbVersion = _UsbVersion,
                .Class = _Class,
                .SubClass = _SubClass,
                .Protocol = _Protocol,
                .MaxPacketSize = _Ep0::MaxPacketSize,
                .VendorId = _VendorId,
                .ProductId = _ProductId,
                .DeviceReleaseNumber = _DeviceReleaseNumber,
                .ManufacturerStringIndex = (std::is_same_v<decltype(_Manufacturer), decltype(EmptyFixedString16)>) ? 0 : 1,
                .ProductStringIndex = (std::is_same_v<decltype(_Product), decltype(EmptyFixedString16)>) ? 0 : 2,
                .SerialNumberStringIndex = (std::is_same_v<decltype(_Serial), decltype(EmptyFixedString16)>) ? 0 : 3,
                .ConfigurationsCount = sizeof...(_Configurations)
            };
        }

        /**
         * @brief Common USB handler
         *
         * @par Returns
         *  Nothing
         */
        static void CommonHandler()
        {
            NVIC_ClearPendingIRQ(_IRQNumber);

            if(_Regs()->ISTR & USB_ISTR_RESET)
            {
                Reset();
            }
            if (_Regs()->ISTR & USB_ISTR_CTR)
            {
                uint8_t endpoint = _Regs()->ISTR & USB_ISTR_EP_ID;
                EpHandlers::Handle(endpoint, ((_Regs()->ISTR & USB_ISTR_DIR) != 0 ? EndpointDirection::Out : EndpointDirection::In));
            }
        }

        /**
         * @brief Reset device
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            _Ep0::Reset();
            
            (_Configurations::Reset(), ...);

            _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
            _Regs()->ISTR = 0;
            _Regs()->BTABLE = 0;
            _Regs()->DADDR = USB_DADDR_EF;
        }

        /**
         * @brief Ep0 (device) CTR handler
         * 
         * @par Returns
         *  Nothing
         */
        static void Handler()
        {
            if(_Ep0::Reg::Get() & USB_EP_CTR_RX)
            {
                _Ep0::ClearCtrRx();
                
                if(_Ep0::Reg::Get() & USB_EP_SETUP)
                {
                    SetupPacket* setup = reinterpret_cast<SetupPacket*>(_Ep0::RxBuffer);

                    if(setup->RequestType.Recipient == 1)
                    {
                        IfHandlers::HandleSetupRequest(setup->Index & 0xff);
                        return;
                    }

                    switch (setup->Request) {
                    case StandartRequestCode::GetStatus: {
                        uint16_t status = 0;
                        _Ep0::SendData(&status, sizeof(status));
                        break;
                    }
                    case StandartRequestCode::SetAddress: {
                        _tempAddressStorage = setup->Value;                        
                        _Ep0::SendZLP([](){
                            _Regs()->DADDR = (USB_DADDR_EF | (_tempAddressStorage & USB_DADDR_ADD));
                            _Ep0::SetRxStatus(EndpointStatus::Valid);
                        });
                        break;
                    }
                    case StandartRequestCode::GetDescriptor: {
                        switch (static_cast<GetDescriptorParameter>(setup->Value)) {
                        case GetDescriptorParameter::DeviceDescriptor: {
                            DeviceDescriptor tempDeviceDescriptor;
                            FillDescriptor(reinterpret_cast<DeviceDescriptor*>(&tempDeviceDescriptor));
                            _Ep0::SendData(&tempDeviceDescriptor, setup->Length < sizeof(DeviceDescriptor) ? setup->Length : sizeof(DeviceDescriptor));
                            break;
                        }
                        case GetDescriptorParameter::ConfigurationDescriptor: {
                            uint8_t temp[128];
                            // Now supports only one configuration. It will easy to support more by adding dispatcher like in endpoint/interface
                            uint16_t size = GetType<0, Configurations>::type::FillDescriptor(reinterpret_cast<ConfigurationDescriptor*>(&temp[0]));
                            _Ep0::SendData(reinterpret_cast<ConfigurationDescriptor*>(&temp[0]), setup->Length < size ? setup->Length : size);
                            break;
                        }
                        case GetDescriptorParameter::StringLangDescriptor: {
                            LangIdDescriptor langIdDescriptor;
                            _Ep0::SendData(&langIdDescriptor, setup->Length < sizeof(langIdDescriptor) ? setup->Length : sizeof(langIdDescriptor));
                            break;
                        }
                        case GetDescriptorParameter::StringManDescriptor: {
                            if constexpr (!std::is_same_v<decltype(_Manufacturer), decltype(EmptyFixedString16)>)
                            {
                                uint8_t temp[sizeof(StringDescriptor) + _Manufacturer.Size];
                                auto descriptor = reinterpret_cast<StringDescriptor*>(temp);
                                *descriptor = StringDescriptor {.Length = sizeof(temp)};

                                memcpy(descriptor->String, _Manufacturer.Text, _Manufacturer.Size);
                                _Ep0::SendData(temp, setup->Length < descriptor->Length ? setup->Length : descriptor->Length);
                                break;
                            }
                        }

                        case GetDescriptorParameter::StringProdDescriptor: {
                            if constexpr (!std::is_same_v<decltype(_Product), decltype(EmptyFixedString16)>)
                            {
                                uint8_t temp[sizeof(StringDescriptor) + _Product.Size];
                                auto descriptor = reinterpret_cast<StringDescriptor*>(temp);
                                *descriptor = StringDescriptor {.Length = sizeof(temp)};

                                memcpy(descriptor->String, _Product.Text, _Product.Size);
                                _Ep0::SendData(temp, setup->Length < descriptor->Length ? setup->Length : descriptor->Length);
                                break;
                            }
                        }
                        case GetDescriptorParameter::StringSerialNumberDescriptor: {
                            if constexpr (!std::is_same_v<decltype(_Serial), decltype(EmptyFixedString16)>)
                            {
                                uint8_t temp[sizeof(StringDescriptor) + _Serial.Size];
                                auto descriptor = reinterpret_cast<StringDescriptor*>(temp);
                                *descriptor = StringDescriptor {.Length = sizeof(temp)};

                                memcpy(descriptor->String, _Serial.Text, _Serial.Size);
                                _Ep0::SendData(temp, setup->Length < descriptor->Length ? setup->Length : descriptor->Length);
                                break;
                            }
                        }
                        default:
                            _Ep0::SetTxStatus(EndpointStatus::Stall);
                            break;
                        }
                        break;
                    }
                    case StandartRequestCode::GetConfiguration: {
                        uint8_t response = _isDeviceConfigured ? 1 : 0;
                        _Ep0::SendData(&response, 1);
                        break;
                    }
                    case StandartRequestCode::SetConfiguration: {
                        _isDeviceConfigured = true;
                        _Ep0::SendZLP();
                        break;
                    }
                    default:
                        _Ep0::SetTxStatus(EndpointStatus::Stall);
                        break;
                    }
                }
                else
                {
                    _Ep0::TryHandleDataTransfer();
                }
            }
            if(_Ep0::Reg::Get() & USB_EP_CTR_TX)
            {
                _Ep0::ClearCtrTx();
                _Ep0::HandleTx();
            }
        }
    };

    IO_STRUCT_WRAPPER(USB, UsbRegs, USB_TypeDef);

    #define USB_DEVICE_TEMPLATE_ARGS template< \
            typename _Regs, \
            IRQn_Type _IRQNumber, \
            typename _ClockCtrl, \
            uint16_t _UsbVersion, \
            DeviceAndInterfaceClass _Class, \
            uint8_t _SubClass, \
            uint8_t _Protocol, \
            uint16_t _VendorId, \
            uint16_t _ProductId, \
            uint16_t _DeviceReleaseNumber, \
            auto _Manufacturer, \
            auto _Product, \
            auto _Serial, \
            typename _Ep0, \
            typename... _Configurations>

    #define USB_DEVICE_TEMPLATE_QUALIFIER DeviceBase<_Regs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>

    USB_DEVICE_TEMPLATE_ARGS
    uint8_t USB_DEVICE_TEMPLATE_QUALIFIER::_tempAddressStorage = 0x00;

    USB_DEVICE_TEMPLATE_ARGS
    volatile bool USB_DEVICE_TEMPLATE_QUALIFIER::_isDeviceConfigured = false;
}
#endif // ZHELE_USB_DEVICE_H