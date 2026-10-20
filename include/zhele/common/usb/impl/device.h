/**
 * @file
 * Implement USB device class methods
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_COMMON_USB_IMPL_DEVICE_H
#define ZHELE_COMMON_USB_IMPL_DEVICE_H

namespace Zhele::Usb
{
    #define USB_DEVICE_TEMPLATE_ARGS template< \
        typename _Controller, \
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
    #define USB_DEVICE_TEMPLATE_QUALIFIER DeviceBase<_Controller, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Enable()
    {
        _Controller::template Enable<This>();
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Reset()
    {
        _Controller::template Reset<This>();
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::CommonHandler()
    {
        _Controller::template CommonHandler<This>();
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::Handler()
    {
        _Controller::template Ep0Handler<This>();
    }

    USB_DEVICE_TEMPLATE_ARGS
    bool USB_DEVICE_TEMPLATE_QUALIFIER::IsDeviceConfigured()
    {
        return _isDeviceConfigured;
    }

    USB_DEVICE_TEMPLATE_ARGS
    consteval DeviceDescriptor USB_DEVICE_TEMPLATE_QUALIFIER::BuildDeviceDescriptor()
    {
        return DeviceDescriptor {
            .UsbVersion = _UsbVersion,
            .Class = _Class,
            .SubClass = _SubClass,
            .Protocol = _Protocol,
            .MaxPacketSize = _Ep0::MaxPacketSize,
            .VendorId = _VendorId,
            .ProductId = _ProductId,
            .DeviceReleaseNumber = _DeviceReleaseNumber,
            .ManufacturerStringIndex = (std::is_same_v<decltype(_Manufacturer), decltype(template_utils::EmptyFixedString16)>) ? 0 : 1,
            .ProductStringIndex = (std::is_same_v<decltype(_Product), decltype(template_utils::EmptyFixedString16)>) ? 0 : 2,
            .SerialNumberStringIndex = (std::is_same_v<decltype(_Serial), decltype(template_utils::EmptyFixedString16)>) ? 0 : 3,
            .ConfigurationsCount = sizeof...(_Configurations)
        };
    }

    USB_DEVICE_TEMPLATE_ARGS
    consteval unsigned USB_DEVICE_TEMPLATE_QUALIFIER::ConfigurationDescriptorSize()
    {
        uint16_t size = 0;

        _configurations.foreach([&size](auto configuration) {
            size += template_utils::type_unbox<configuration>::GetDescriptor().size();
        });

        return size;
    }


    USB_DEVICE_TEMPLATE_ARGS
    consteval auto USB_DEVICE_TEMPLATE_QUALIFIER::BuildConfigurationDescriptor()
    {
        constexpr auto size = ConfigurationDescriptorSize();

        std::array<uint8_t, size> result;
        auto dst = result.begin();

        _configurations.foreach([&dst](auto configuration) {
            auto nextConfigurationDescriptor = template_utils::type_unbox<configuration>::GetDescriptor();
            dst = std::copy(nextConfigurationDescriptor.begin(), nextConfigurationDescriptor.end(), dst);
        });

        return result;
    }

    USB_DEVICE_TEMPLATE_ARGS
    inline consteval auto USB_DEVICE_TEMPLATE_QUALIFIER::BuildStringDescriptor(auto str)
    {
        std::array<uint16_t, str.Size / 2 + 1> result {
            ((2 + str.Size) << 8) | static_cast<uint8_t>(DescriptorType::String)
        };
        auto dst = result.begin();
        ++dst;

        std::copy(str.begin(), str.end(), dst);

        return result;
    }

    USB_DEVICE_TEMPLATE_ARGS
    void USB_DEVICE_TEMPLATE_QUALIFIER::HandleSetupRequest(SetupPacket* setupRequest)
    {
        if (setupRequest->RequestType.Recipient == 1) {
            _ifHandlers.HandleSetupRequest(setupRequest->Index & 0xff);
            return;
        }
        
        switch (setupRequest->Request) {
        case StandartRequestCode::GetStatus: {
            static constexpr uint16_t status = 0;
            _Ep0::SendData(&status, sizeof(status));
            break;
        }
        case StandartRequestCode::SetAddress: {
            _Controller::template SetAddress<This>(setupRequest->Value);
            break;
        }

        case StandartRequestCode::GetDescriptor: {
            switch (static_cast<GetDescriptorParameter>(setupRequest->Value)) {
            case GetDescriptorParameter::DeviceDescriptor: {
                static constexpr auto descriptor = BuildDeviceDescriptor();
                _Ep0::SendData(&descriptor, setupRequest->Length < sizeof(descriptor) ? setupRequest->Length : sizeof(descriptor));
                break;
            }

            case GetDescriptorParameter::ConfigurationDescriptor: {
                static constexpr auto descriptor = BuildConfigurationDescriptor();
                _Ep0::SendData(descriptor.data(), setupRequest->Length < descriptor.size() ? setupRequest->Length : descriptor.size());
                break;
            }
            case GetDescriptorParameter::StringLangDescriptor: {
                static constexpr LangIdDescriptor langIdDescriptor;
                _Ep0::SendData(&langIdDescriptor, setupRequest->Length < sizeof(langIdDescriptor) ? setupRequest->Length : sizeof(langIdDescriptor));
                break;
            }
            case GetDescriptorParameter::StringManDescriptor: {
                if constexpr (!std::is_same_v<decltype(_Manufacturer), decltype(template_utils::EmptyFixedString16)>)
                {
                    static constexpr auto descriptor = BuildStringDescriptor(_Manufacturer);
                    static constexpr auto size = descriptor.size() * sizeof(descriptor[0]);
                    _Ep0::SendData(descriptor.data(), setupRequest->Length < size ? setupRequest->Length : size);
                    break;
                }
            }

            case GetDescriptorParameter::StringProdDescriptor: {
                if constexpr (!std::is_same_v<decltype(_Product), decltype(template_utils::EmptyFixedString16)>)
                {
                    static constexpr auto descriptor = BuildStringDescriptor(_Product);
                    static constexpr auto size = descriptor.size() * sizeof(descriptor[0]);
                    _Ep0::SendData(descriptor.data(), setupRequest->Length < size ? setupRequest->Length : size);
                    break;
                }
            }
            case GetDescriptorParameter::StringSerialNumberDescriptor: {
                if constexpr (!std::is_same_v<decltype(_Serial), decltype(template_utils::EmptyFixedString16)>)
                {
                    static constexpr auto descriptor = BuildStringDescriptor(_Serial);
                    static constexpr auto size = descriptor.size() * sizeof(descriptor[0]);
                    _Ep0::SendData(descriptor.data(), setupRequest->Length < size ? setupRequest->Length : size);
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
            static uint8_t response = _isDeviceConfigured ? 1 : 0;
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

    USB_DEVICE_TEMPLATE_ARGS
    volatile bool USB_DEVICE_TEMPLATE_QUALIFIER::_isDeviceConfigured = false;
}

#endif //! ZHELE_COMMON_USB_IMPL_DEVICE_H