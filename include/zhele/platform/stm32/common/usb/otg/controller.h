/**
 * @file
 * Implement USB device controller for Synopsys OTG core
 *
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_OTG_CONTROLLER_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_OTG_CONTROLLER_H

#include "endpoints_manager.h"

namespace Zhele::Usb
{
    template<typename _Regs>
    void FlushTx()
    {
        while ((_Regs()->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U) continue;

        _Regs()->GRSTCTL = USB_OTG_GRSTCTL_TXFFLSH | (1 << 10);
        while(_Regs()->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) continue;;
    }

    template<typename _Regs>
    void FlushRx()
    {
        while ((_Regs()->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U) continue;
        _Regs()->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;
        while (_Regs()->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH);
    }

    /**
     * @brief Implements USB device controller
     *
     * @tparam _Regs USB global regs wrapper
     * @tparam _DeviceRegs USB device regs wrapper
     * @tparam _IRQNumber Irq number
     * @tparam _ClockCtrl Clock control
     */
    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    class OtgController
    {
        template<typename _Device>
        using BufferManager = decltype(EndpointsManager{_Device::_endpoints.template push_back<typename _Device::Ep0>()});

        /**
         * @brief Calculate DAINTMSK value
         *
         * @param [in] endpoints Endpoints
         *
         * @returns DAINT mask
         */
        static consteval auto CalculateDaintMask(auto endpoints) {
            uint32_t result = 0;

            endpoints.foreach([&result](auto endpoint) {
                result |=  template_utils::type_unbox<endpoint>::Direction == EndpointDirection::In
                    ? (1 << template_utils::type_unbox<endpoint>::Number)
                    : (template_utils::type_unbox<endpoint>::Direction == EndpointDirection::Out
                        ? ((1 << template_utils::type_unbox<endpoint>::Number) << 16)
                        : ((1 << template_utils::type_unbox<endpoint>::Number) | ((1 << template_utils::type_unbox<endpoint>::Number) << 16))
                    );
            });

            return result;
        }
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

    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void OtgController<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl>::Enable()
    {
        _ClockCtrl::Enable();

        BufferManager<_Device>::Init();

        while (!(_Regs()->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL)) continue;

        _DeviceRegs()->DCFG = USB_OTG_DCFG_DSPD;

        _Regs()->GUSBCFG = USB_OTG_GUSBCFG_FDMOD // Force device mode
                        | (0x06 << USB_OTG_GUSBCFG_TRDT_Pos) // ??
                        | USB_OTG_GUSBCFG_PHYSEL; // USB 2.0

        _Regs()->GCCFG = USB_OTG_GCCFG_NOVBUSSENS; // Exit Power Down mode
        _Regs()->GINTSTS = 0xfffffffful;

        _Regs()->GINTMSK = USB_OTG_GINTMSK_IEPINT   // Enable USB IN TX endpoint interrupt
                            | USB_OTG_GINTMSK_OEPINT   // Enable USB OUT RX endpoint interrupt
                            | USB_OTG_GINTMSK_RXFLVLM // USB reciving
                            | USB_OTG_GINTMSK_USBRST
                            | USB_OTG_GINTMSK_ENUMDNEM;    // Reset interrupt

        _Regs()->GCCFG |= USB_OTG_GCCFG_PWRDWN; // Enable USB
        _DeviceRegs()->DCTL = 0;

        NVIC_EnableIRQ(_IRQNumber);
        _Regs()->GAHBCFG = USB_OTG_GAHBCFG_GINT;       // On USB general interrupt
    }

    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void OtgController<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl>::Reset()
    {
        for (auto i = 0; i < 3; ++i)
            _Regs()->DIEPTXF[i] = 0;

        _DeviceRegs()->DCFG &= ~USB_OTG_DCFG_DAD;

        BufferManager<_Device>::Init();

        _Device::ResetEndpoints();

        _DeviceRegs()->DAINTMSK = CalculateDaintMask(_Device::_endpoints.template push_back<_Device>());
        _DeviceRegs()->DOEPMSK = USB_OTG_DOEPMSK_STUPM // Enable setup-done irq
                                | USB_OTG_DOEPMSK_XFRCM;  // Enable tx-done irq

        _DeviceRegs()->DIEPMSK = USB_OTG_DIEPMSK_XFRCM; // Enable transfer complete irq

        FlushTx<_Regs>();
        FlushRx<_Regs>();
    }

    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void OtgController<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl>::CommonHandler()
    {
        static constexpr auto outEndpoints = _Device::_endpoints.filter([](auto endpoint){
            return endpoint.Direction == EndpointDirection::Out || endpoint.Direction == EndpointDirection::Bidirectional;
        }).template push_back<_Device>();
        static constexpr auto epFifoNotEmptyHandlers = EndpointFifoNotEmptyHandlers{outEndpoints};

        if (_Regs()->GINTSTS & USB_OTG_GINTSTS_USBRST) {
            _Regs()->GINTSTS = USB_OTG_GINTSTS_USBRST;
            _Device::Reset();
        }

        if (_Regs()->GINTSTS & USB_OTG_GINTSTS_ENUMDNE) {
            _Regs()->GINTSTS = USB_OTG_GINTSTS_ENUMDNE;
            _Device::_isDeviceConfigured = true;
        }

        if (_Regs()->GINTSTS & USB_OTG_GINTSTS_RXFLVL) {
            uint32_t status = _Regs()->GRXSTSP;
            uint16_t size = (status & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;
            uint8_t enpointNumber = status & USB_OTG_GRXSTSP_EPNUM;

            epFifoNotEmptyHandlers.HandleRxFifoNotEmpty(enpointNumber, size);
        }

        if(_Regs()->GINTSTS & USB_OTG_GINTSTS_OEPINT) {
            uint32_t endpoints = _DeviceRegs()->DAINT & _DeviceRegs()->DAINTMSK;

            if (endpoints & (1 << 16)) {
                _Device::_epHandlers.Handle(0, EndpointDirection::Out);
            }
            if (endpoints & (1 << 17)) {
                _Device::_epHandlers.Handle(1, EndpointDirection::Out);
            }
            if (endpoints & (1 << 18)) {
                _Device::_epHandlers.Handle(2, EndpointDirection::Out);
            }
            if (endpoints & (1 << 19)) {
                _Device::_epHandlers.Handle(3, EndpointDirection::Out);
            }
        }

        if(_Regs()->GINTSTS & (USB_OTG_GINTSTS_IEPINT | USB_OTG_GINTSTS_NPTXFE)) {
            uint32_t endpoints = _DeviceRegs()->DAINT & _DeviceRegs()->DAINTMSK;

            if (endpoints & (1 << 0)) {
                _Device::_epHandlers.Handle(0, EndpointDirection::In);
            }
            if (endpoints & (1 << 1)) {
                _Device::_epHandlers.Handle(1, EndpointDirection::In);
            }
            if (endpoints & (1 << 2)) {
                _Device::_epHandlers.Handle(2, EndpointDirection::In);
            }
            if (endpoints & (1 << 3)) {
                _Device::_epHandlers.Handle(3, EndpointDirection::In);
            }
        }
    }

    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void OtgController<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl>::Ep0Handler()
    {
        using Ep0 = typename _Device::Ep0;

        if (Ep0::GetOutInterrupts() & USB_OTG_DOEPINT_STUP) {
            _Device::HandleSetupRequest(reinterpret_cast<SetupPacket*>(Ep0::RxBuffer));
            Ep0::SetRxStatus(EndpointStatus::Valid);
        }
        if (Ep0::GetOutInterrupts() & USB_OTG_DOEPINT_XFRC) {
            Ep0::TryHandleDataTransfer();
        }

        Ep0::ClearAllRxInterrupts();

        Ep0::HandleTx();
    }

    template<typename _Regs, typename _DeviceRegs, IRQn_Type _IRQNumber, typename _ClockCtrl>
    template<typename _Device>
    void OtgController<_Regs, _DeviceRegs, _IRQNumber, _ClockCtrl>::SetAddress(uint16_t address)
    {
        _DeviceRegs()->DCFG = (_DeviceRegs()->DCFG & ~USB_OTG_DCFG_DAD) | (static_cast<uint32_t>(address) << USB_OTG_DCFG_DAD_Pos);
        _Device::Ep0::SendZLP();
    }
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_OTG_CONTROLLER_H
