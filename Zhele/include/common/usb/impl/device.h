/**
 * @file
 * Implement USB device class methods
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_DEVICE_IMPL_H
#define ZHELE_USB_DEVICE_IMPL_H

namespace Zhele::Usb
{
#if defined (USB)
    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Init()
    {
        using EpBufferManager = EndpointsManager<Append_t<_Ep0, Endpoints>>;

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

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Reset()
    {
        _Ep0::Reset();
        
        (_Configurations::Reset(), ...);

        _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
        _Regs()->ISTR = 0;
        _Regs()->BTABLE = 0;
        _Regs()->DADDR = USB_DADDR_EF;
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::CommonHandler()
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
#endif
    USB_DEVICE_TEMPLATE_ARGS
    bool USB_DEVICE_TEMPLATE_QUALIFIER::IsDeviceConfigured()
    {
        return _isDeviceConfigured;
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::FillDescriptor(DeviceDescriptor* descriptor)
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

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Handler()
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

#if defined (USB)
    IO_STRUCT_WRAPPER(USB, UsbRegs, USB_TypeDef);
#elif defined (USB_OTG_FS)
    IO_STRUCT_WRAPPER(USB_OTG_FS, UsbRegs, USB_OTG_GlobalTypeDef);
    IO_STRUCT_WRAPPER(USB_OTG_FS + USB_OTG_DEVICE_BASE, UsbDeviceRegs, USB_OTG_DeviceTypeDef);
#endif

#if defined (USB)
    #define USB_DEVICE_TEMPLATE_ARGS template< \
        typename _Regs, \
        typename _DeviceRegs, \
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
#elif defined (USB_OTG_FS)
    #define USB_DEVICE_TEMPLATE_ARGS template< \
        typename _Regs, \
        typename _DeviceRegs, \
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
    #define USB_DEVICE_TEMPLATE_QUALIFIER DeviceBase<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>
#endif

#if defined (USB_IRQn)
    #define USB_IRQ USB_IRQn
#elif defined (USB_LP_IRQn)
    #define USB_IRQ USB_LP_IRQn
#elif defined (OTG_FS_IRQn)
    #define USB_IRQ OTG_FS_IRQn
#endif

#if defined (USB)
    using UsbClock = Zhele::Clock::UsbClock;
#elif defined (USB_OTG_FS)
    using UsbClock = Zhele::Clock::OtgFsClock;
#endif
    template<
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
#if defined (USB)
    using DeviceWithStrings = DeviceBase<UsbRegs, USB_IRQ, UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;
#elif defined (USB_OTG_FS)
    using DeviceWithStrings = DeviceBase<UsbRegs, UsbDeviceRegs, USB_IRQ, UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;
#endif
    template<
        uint16_t _UsbVersion,
        DeviceAndInterfaceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        typename _Ep0,
        typename... _Configurations>
#if defined (USB)
    using Device = DeviceBase<UsbRegs, USB_IRQ, UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, EmptyFixedString16, EmptyFixedString16, EmptyFixedString16, _Ep0, _Configurations...>;
#elif defined (USB_OTG_FS)
    using Device = DeviceBase<UsbRegs, UsbDeviceRegs, USB_IRQ, UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, EmptyFixedString16, EmptyFixedString16, EmptyFixedString16, _Ep0, _Configurations...>;
#endif

    USB_DEVICE_TEMPLATE_ARGS
    uint8_t USB_DEVICE_TEMPLATE_QUALIFIER::_tempAddressStorage = 0x00;

    USB_DEVICE_TEMPLATE_ARGS
    volatile bool USB_DEVICE_TEMPLATE_QUALIFIER::_isDeviceConfigured = false;
}

#endif //! ZHELE_USB_DEVICE_IMPL_H