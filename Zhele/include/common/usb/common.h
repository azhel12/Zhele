/**
 * @file
 * Implement USB common code
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_COMMON_H
#define ZHELE_USB_COMMON_H

#include <stdint.h>

namespace Zhele::Usb
{
    /**
     * @brief Decription type constant
     */
    enum class DescriptorType : uint8_t
    {
        Device = 0x01, //< Device descriptor
        Configuration = 0x02, //< Configuration descriptor
        String = 0x03, //< String descriptor
        Interface = 0x04, //< Interface descriptor
        Endpoint = 0x05 //< Endpoint descriptor
    };

    /**
     * @brief Device class
     */
    enum class DeviceClass : uint8_t
    {
        InterfaceSpecified = 0x00,
        Audio = 0x01,
        Comm = 0x02,
        Hid = 0x03,
        Monitor = 0x04,
        Physic = 0x05,
        Power = 0x06,
        Printer = 0x07,
        Storage = 0x08,
        Hub = 0x09,
        VendorSpecified = 0xff
    };

    /**
     * @brief USB Standard Request Codes
     */
    enum class StandartRequestCode : uint8_t
    {
        GetStatus = 0x00,
        ClearFeature = 0x01,
        SetFeature = 0x03,
        SetAddress = 0x05,
        GetDescriptor = 0x06,
        SetDescriptor = 0x07,
        GetConfiguration = 0x08,
        SetConfiguration = 0x09,
        GetInterface = 0x0a,
        SetInterface = 0x0b,
        SyncFrame = 0x0c,
    };

    /**
     * @brief GET_DESCRIPTOR parameter
     */
    enum class GetDescriptorParameter : uint16_t
    {
        DeviceDescriptor = 0x100,
        ConfigurationDescriptor = 0x200,
        HidReportDescriptor = 0x2200,
        StringLangDescriptor = 0x300,
        StringManDescriptor = 0x301,
        StringProdDescriptor = 0x302,
        StringSerialNumberDescriptor = 0x303,
        DeviceQalifierDescriptor = 0x600,
    };

    /**
     * @brief Setup packet request type
     */
    struct SetupRequestType
    {
        uint8_t Recipient : 5;
        uint8_t Type : 2;
        uint8_t Dir : 1;
    };

    /**
     * @brief Setup packet.
     */
    struct SetupPacket
    {
        SetupRequestType RequestType;
        StandartRequestCode Request;
        uint16_t Value;
        uint16_t Index;
        uint16_t Length;
    };
}
#endif // ZHELE_USB_COMMON_H