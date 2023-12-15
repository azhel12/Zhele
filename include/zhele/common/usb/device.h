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
#include "cdc.h"
#include "endpoints_manager.h"
#include "hid.h"
#include "interface.h"
#include "msc.h"

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
#if defined (USB_OTG_FS)
        typename _DeviceRegs,
#endif
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

#if defined (USB)
        using This = DeviceBase<_Regs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;
#elif defined (USB_OTG_FS)
        using This = DeviceBase<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;
#endif
        static constexpr auto _configurations = TypeList<_Configurations...>{};
        static constexpr auto _interfaces = (TypeList<>{} + ... + _Configurations::Interfaces); 
        static constexpr auto _endpoints = (TypeList<>{} + ... + _Configurations::Endpoints);
        static constexpr auto _epBufferManager = EndpointsManager{_endpoints.template push_back<_Ep0>()};
        static constexpr auto _epHandlers = EndpointHandlers{_endpoints.template push_back<This>()}; // Replace Ep0 with this for correct handler register.
        static constexpr auto _ifHandlers = InterfaceHandlers{(TypeList<>{} + ... + _Configurations::Interfaces)};
#if defined (USB_OTG_FS)
        static constexpr auto _outEndpoints = _endpoints.filter([](auto endpoint){
            return endpoint.Direction == EndpointDirection::Out || endpoint.Direction == EndpointDirection::Bidirectional;
        }).template push_back<This>();
        static constexpr auto _epFifoNotEmptyHandlers = EndpointFifoNotEmptyHandlers{_outEndpoints};
#endif

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
         * @brief Handle SETUP request
         * 
         * @param setupPacket 
         * 
         * @par Returns
         *  Nohting
         */
        static void HandleSetupRequest(SetupPacket* setupPacket);

        /**
         * @brief Set device address
         * 
         * @param address Address
         * 
         * @par Returns
         *  Nothing
         */
        static void SetAddress(uint16_t address);

        /**
         * @brief Calculate DAINTMSK value for OTG
         * 
         * @param [in] endpoints Endpoints
         * 
         * @returns DAINT mask
         */
        static consteval auto CalculateDaintMask(auto endpoints) {
            uint32_t result = 0;

            endpoints.foreach([&result](auto endpoint) {
                result |=  TypeUnbox<endpoint>::Direction == EndpointDirection::In
                    ? (1 << TypeUnbox<endpoint>::Number)
                    : (TypeUnbox<endpoint>::Direction == EndpointDirection::Out
                        ? ((1 << TypeUnbox<endpoint>::Number) << 16)
                        : ((1 << TypeUnbox<endpoint>::Number) | ((1 << TypeUnbox<endpoint>::Number) << 16))
                    );
            });

            return result;
        }
    };
}

#include "impl/device.h"

#endif // ZHELE_USB_DEVICE_H