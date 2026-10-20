/**
 * @file
 * Implement USB device controller for PMA-based peripheral (incl. USB_DRD_FS)
 *
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_PMA_CONTROLLER_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_PMA_CONTROLLER_H

#include "endpoints_manager.h"

namespace Zhele::Usb
{
    /**
     * @brief Implements USB device controller
     *
     * @tparam _Regs USB regs wrapper
     * @tparam _IRQNumber Irq number
     * @tparam _ClockCtrl Clock control
     */
    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    class PmaController
    {
        template<typename _Device>
        using BufferManager = decltype(EndpointsManager{_Device::_endpoints.template push_back<typename _Device::Ep0>()});

        static inline uint8_t _tempAddressStorage = 0x00;
    public:
        template<typename _Device>
        static void Enable();

        template<typename _Device>
        static void Reset();

        template<typename _Device>
        static void CommonHandler();

        template<typename _Device>
        static void Ep0Handler();

        template<typename _Device>
        static void SetAddress(uint16_t address);

    };

    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void PmaController<_Regs, _IRQNumber, _ClockCtrl>::Enable()
    {
        _ClockCtrl::Enable();
#if defined (ZHELE_USB_DRD)
        // Both PDWN and USBRST are set after power-on. Leave power down first,
        // wait for the transceiver startup time and only then release the reset.
        _Regs()->CNTR = USB_CNTR_USBRST;
        for(unsigned i = 0; i < 100; ++i) __NOP(); // tSTARTUP
        _Regs()->CNTR = 0;
#endif
        BufferManager<_Device>::Init();

        _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
        _Regs()->ISTR = 0;
#if !defined (ZHELE_USB_DRD)
        _Regs()->BTABLE = 0; // DRD has no BTABLE register, the table always starts at the packet memory base
#endif
#if defined (USB_BCDR_DPPU)
        _Regs()->BCDR |= USB_BCDR_DPPU;
#endif
        NVIC_EnableIRQ(_IRQNumber);
    }

    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void PmaController<_Regs, _IRQNumber, _ClockCtrl>::Reset()
    {
        _Device::ResetEndpoints();

        _Regs()->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
        _Regs()->ISTR = 0;
#if !defined (ZHELE_USB_DRD)
        _Regs()->BTABLE = 0;
#endif
        _Regs()->DADDR = USB_DADDR_EF;
    }

    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void PmaController<_Regs, _IRQNumber, _ClockCtrl>::CommonHandler()
    {
        NVIC_ClearPendingIRQ(_IRQNumber);

        if(_Regs()->ISTR & USB_ISTR_RESET)
        {
            _Device::Reset();
        }
        if (_Regs()->ISTR & USB_ISTR_CTR)
        {
            uint8_t endpoint = _Regs()->ISTR & USB_ISTR_EP_ID;
            _Device::_epHandlers.Handle(endpoint, ((_Regs()->ISTR & USB_ISTR_DIR) != 0 ? EndpointDirection::Out : EndpointDirection::In));
        }
    }

    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void PmaController<_Regs, _IRQNumber, _ClockCtrl>::Ep0Handler()
    {
        using Ep0 = typename _Device::Ep0;

        if(Ep0::Reg::Get() & USB_EP_CTR_RX)
        {
            Ep0::ClearCtrRx();
            Ep0::FetchRxBuffer();

            if(Ep0::Reg::Get() & USB_EP_SETUP)
            {
                _Device::HandleSetupRequest(reinterpret_cast<SetupPacket*>(Ep0::RxBuffer));
            }
            else
            {
                Ep0::TryHandleDataTransfer();
            }
        }
        if(Ep0::Reg::Get() & USB_EP_CTR_TX)
        {
            Ep0::ClearCtrTx();
            Ep0::HandleTx();
        }
    }

    template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void PmaController<_Regs, _IRQNumber, _ClockCtrl>::SetAddress(uint16_t address)
    {
        using Ep0 = typename _Device::Ep0;
        _tempAddressStorage = address;
        Ep0::SendZLP([](){
            _Regs()->DADDR = (USB_DADDR_EF | (_tempAddressStorage & USB_DADDR_ADD));
            Ep0::SetRxStatus(EndpointStatus::Valid);
        });
    }
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_PMA_CONTROLLER_H
