/**
 * @file
 * Implement USB device
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_COMMON_USB_DEVICE_H
#define ZHELE_COMMON_USB_DEVICE_H

#include "configuration.h"
#include "cdc.h"
#include "endpoint_handlers.h"
#include "hid.h"
#include "interface.h"
#include "msc.h"

#include <zhele/common/template_utils/fixed_string.h>

#include <type_traits>

namespace Zhele::Usb
{
    /**
     * @brief Device descriptor
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
     * @brief Implements USB device
     * 
     * @details Descriptors and standard requests are handled here, everything touching
     * the hardware is delegated to _Controller, which must provide:
     *  - Enable<Device>(), Reset<Device>(), CommonHandler<Device>()
     *  - Ep0Handler<Device>() (Ep0 interrupt: detect SETUP and call Device::HandleSetupRequest)
     *  - SetAddress<Device>(address)
     * 
     * @tparam _Controller Platform USB controller
     * @tparam _UsbVersion Usb version
     * @tparam _Class Class
     * @tparam _SubClass Subclass
     * @tparam _Protocol Protocol
     * @tparam _VendorId Vendor ID
     * @tparam _ProductId Product ID
     * @tparam _DeviceReleaseNumber Device release number
     * @tparam _Manufacturer Manufacturer string
     * @tparam _Product Product string
     * @tparam _Serial Serial number string
     * @tparam _Ep0 Endpoint 0 base
     * @tparam _Configurations Device`s configurations
     */
    template<
        typename _Controller,
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
        friend _Controller;

        /**
         * @brief Build device descriptor
         * 
         * @returns Device descriptor
         */
        static consteval DeviceDescriptor BuildDeviceDescriptor();

        /**
         * @brief Calculates configuration descriptor size
         * 
         * @returns Configuration descriptor size in bytes
        */
        static consteval unsigned ConfigurationDescriptorSize();

        /**
         * @brief Build configuration descriptor
         * 
         * @returns Bytes with configuration descriptor
        */
        static consteval auto BuildConfigurationDescriptor();

        /**
         * @brief Build string descriptor
         * 
         * @returns Bytes with string descriptor
        */
        static consteval auto BuildStringDescriptor(auto str);

        using This = DeviceBase<_Controller, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;
        using Ep0 = _Ep0;

        static constexpr auto _configurations = template_utils::type_list<_Configurations...>{};
        static constexpr auto _endpoints = (template_utils::type_list<>{} + ... + _Configurations::Endpoints);
        static constexpr auto _epHandlers = EndpointHandlers{_endpoints.template push_back<This>()}; // Replace Ep0 with this for correct handler register.
        static constexpr auto _ifHandlers = InterfaceHandlers{(template_utils::type_list<>{} + ... + _Configurations::Interfaces)};

        static volatile bool _isDeviceConfigured;
    public:
        /**
         * @brief Select clock source
         * 
         * @tparam clockSource Clock source (platform-specific ClockSource enum value)
         * 
         * @par Returns
         *  Nothing
         */
        template<auto clockSource>
        static void SelectClockSource();

        /**
         * @brief Enables device
         * 
         * @par Returns
         *  Nothing
         */
        static void Enable();

        /**
         * @brief Reset device
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset();

        /**
         * @brief Allows check that device configure complete
         * 
         * @retval true Device is configured
         * @retval false Device is not configured yet
         */
        static bool IsDeviceConfigured();

        /**
         * @brief Common USB handler
         *
         * @par Returns
         *  Nothing
         */
        static void CommonHandler();

        /**
         * @brief Ep0 (device) CTR handler
         * 
         * @par Returns
         *  Nothing
         */
        static void Handler();

    private:
        /**
         * @brief Reset Ep0 and all configurations endpoints
         * 
         * @par Returns
         *  Nothing
         */
        static void ResetEndpoints()
        {
            _Ep0::Reset();
            (_Configurations::Reset(), ...);
        }

        /**
         * @brief Handle SETUP request
         * 
         * @param setupPacket 
         * 
         * @par Returns
         *  Nohting
         */
        static void HandleSetupRequest(SetupPacket* setupPacket);
    };
}

#include "impl/device.h"

#endif // ZHELE_COMMON_USB_DEVICE_H