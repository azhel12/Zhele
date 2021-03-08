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

namespace Zhele::Usb
{
    /**
     * @brief Device descriptor.
     */
    struct DeviceDescriptor
    {
        static const uint8_t Length = 18;
        static const DescriptorType Type = DescriptorType::Device;
        uint16_t UsbVersion;
        DeviceClass Class;
        uint8_t SubClass;
        uint8_t Protocol;
        uint8_t MaxPacketSize;
        uint16_t VendorId;
        uint16_t ProductId;
        uint16_t DeviceReleaseNumber;
        uint8_t ManufacturerStringIndex = 0;
        uint8_t ProductStringIndex = 0;
        uint8_t SerialNumberStringIndex = 0;
        uint8_t ConfigurationsCount;
    };
    
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
        DeviceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        typename _Ep0,
        typename... _Configurations>
    class DeviceBase : public _Ep0
    {
        using This = DeviceBase<_Regs, _IRQNumber, _ClockCtrl, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Ep0, _Configurations...>;
        using Endpoints = Append_t<typename _Configurations::Endpoints...>;

        // Replace Ep0 with this for correct handler register.
        using AllEndpoints = Append_t<This, Endpoints>;
        using EpBufferManager = EndpointsManager<AllEndpoints>;
        using EpHandlers = EndpointHandlers<AllEndpoints>;
    public:

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
                .ConfigurationsCount = sizeof...(_Configurations)
            };
        }

        static void CommonHandler()
        {
            if(_Regs()->ISTR & USB_ISTR_RESET)
            {
                Reset();
            }
            if(_Regs()->ISTR & USB_ISTR_CTR)
            {
                uint8_t endpoint = _Regs()->ISTR & USB_ISTR_EP_ID;
                EpHandlers::Handle(endpoint, ((_Regs()->ISTR & USB_ISTR_DIR) != 0 ? EndpointDirection::In : EndpointDirection::Out));
                _Regs()->ISTR &= USB_CLR_CTR;
            }

            NVIC_ClearPendingIRQ(_IRQNumber);
        }

        static void Reset()
        {
            _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
            _Regs()->ISTR = 0;

            _Ep0::Reset();
            _Ep0::Reg::Xor( USB_EP_RX_VALID | USB_EP_TX_NAK);
            (_Configurations::Reset(), ...);

            _Regs()->BTABLE = 0;
            _Regs()->DADDR = USB_DADDR_EF;
        }

        static void Handler()
        {
            if(_Ep0::Reg::Get() & USB_EP_SETUP)
            {
                SetupPacket* setup = reinterpret_cast<SetupPacket*>(_Ep0::RxBuffer);
                switch (setup->Request)
                {
                case StandartRequestCode::SetAddress:
                    SetAddress(setup->Value);
                    break;
                case StandartRequestCode::GetDescriptor:
                    switch (static_cast<GetDescriptorParameter>(setup->Value))
                    {
                    case GetDescriptorParameter::DeviceDescriptor:
                        FillDescriptor(reinterpret_cast<DeviceDescriptor*>(_Ep0::TxBuffer));
                        _Ep0::Writer::SendData(sizeof(DeviceDescriptor));
                        break;
                    default:
                        break;
                    }
                case StandartRequestCode::GetStatus:
                    break;
                break;
                }
            }
        }
    private:
        static void SetAddress(uint8_t address)
        {
            _Regs()->DADDR = USB_DADDR_EF | (address & USB_DADDR_ADD);
        }
    };

    IO_STRUCT_WRAPPER(USB, UsbRegs, USB_TypeDef);
    template<
        uint16_t _UsbVersion,
        DeviceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        typename _Ep0,
        typename... _Configurations>
    using Device = DeviceBase<UsbRegs, USB_IRQn, Zhele::Clock::UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Ep0, _Configurations...>;
}
#endif // ZHELE_USB_DEVICE_H