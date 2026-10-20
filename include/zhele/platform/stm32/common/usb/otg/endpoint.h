/**
 * @file
 * Implement USB endpoint for Synopsys OTG core
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINT_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINT_H

#include <zhele/common/usb/endpoint.h>

#include "../../ioreg.h"

#include <cstddef>

namespace Zhele::Usb
{
    /**
     * @brief Implements endpoint
     * 
     * @tparam _Base Base endpoint (EndpointBase specialization)
     * @tparam _Reg EPnR register
     */
    template <typename _Base>
    class Endpoint : public _Base
    {
    public:        
        static const uint16_t Number = _Base::Number;
        static const EndpointDirection Direction = _Base::Direction;
        static const EndpointType Type = _Base::Type;
        static const uint16_t MaxPacketSize = _Base::MaxPacketSize;
        static const uint8_t Interval = _Base::Interval;

        /**
         * @brief Endpoint interrupt handler
         *
         * @par Returns
         *  Nothing
        */
        static void Handler();
    private:
    };

    /**
     * @brief Calculates MPSIZ bits value for zero endpoint
     * 
     * @param maxPacketSize Max packet size in bytes
     * @return constexpr uint8_t MPSIZE value
     */
    constexpr uint8_t CalculateMpsizValueForZeroEndpoint(uint16_t maxPacketSize)
    {
        return maxPacketSize == 8
            ? 0b11
            : maxPacketSize == 16
                ? 0b10
                : maxPacketSize == 32
                    ? 0b01
                    : 0b00;
    }

    /**
     * @brief Implements out (RX) endpoint
     * 
     * @tparam _Base Endpoint base
     * @tparam _Regs EP registers wrapper
     * @tparam _FifoAddress RX buffer address
     */
    template<typename _Base, typename _Regs, uint32_t _FifoAddress>
    class OutEndpoint : public Endpoint<_Base>
    {
    public:
        static uint8_t BufferSize;
        static uint8_t Buffer[_Base::MaxPacketSize];

        /**
         * @brief Received bytes count (same interface as PMA endpoint`s RxBufferCount)
         */
        struct RxBufferCount
        {
            static uint16_t Get() { return BufferSize; }
        };

        /**
         * @brief Reset endpoint
         */
        static void Reset()
        {
            if constexpr (_Base::Number == 0)
            {
                static_assert(_Base::MaxPacketSize == 8 || _Base::MaxPacketSize == 16 || _Base::MaxPacketSize == 32 || _Base::MaxPacketSize == 64);

                _Regs()->DOEPTSIZ = (0b11 << USB_OTG_DOEPTSIZ_STUPCNT_Pos)
                    | (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos)
                    | (_Base::MaxPacketSize << USB_OTG_DOEPTSIZ_XFRSIZ_Pos);

                _Regs()->DOEPCTL = USB_OTG_DOEPCTL_EPENA 
                    | USB_OTG_DOEPCTL_CNAK
                    | USB_OTG_DOEPCTL_USBAEP
                    | CalculateMpsizValueForZeroEndpoint(_Base::MaxPacketSize);
            }
            else
            {
                _Regs()->DOEPTSIZ = (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos)
                    | (_Base::MaxPacketSize << USB_OTG_DOEPTSIZ_XFRSIZ_Pos);
                _Regs()->DOEPCTL = USB_OTG_DOEPCTL_EPENA 
                    | USB_OTG_DOEPCTL_CNAK
                    | (static_cast<uint32_t>(_Base::Type) << USB_OTG_DOEPCTL_EPTYP_Pos)
                    | USB_OTG_DOEPCTL_USBAEP
                    | _Base::MaxPacketSize;
            }
        }

        /**
         * @brief RX fifo not empty handler
         */
        static void Handler()
        {
            HandleRx();
        }

        /**
         * @brief Rx handler
         */
        static void HandleRx();

        /**
         * @brief Set endpoint`s RX status
         *
         * @param [in] status New status
         *
         * @par Returns
         *  Nothing
        */
        static void SetRxStatus(EndpointStatus status)
        {
            switch (status)
            {
            case EndpointStatus::Disable:
                if (_Regs()->DOEPCTL & USB_OTG_DOEPCTL_EPENA) {
                    _Regs()->DOEPCTL |= USB_OTG_DOEPCTL_EPDIS;
                }
                break;
            case EndpointStatus::Stall:
                _Regs()->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
                break;
            case EndpointStatus::Nak:
                _Regs()->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
                break;
            case EndpointStatus::Valid:
                BufferSize = 0;
                _Regs()->DOEPTSIZ = (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos)
                    | (_Base::MaxPacketSize << USB_OTG_DOEPTSIZ_XFRSIZ_Pos);
                _Regs()->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
                break;
            }
        }

        /**
         * @brief Set endpoint`s RX status valid
         */
        static void SetRxStatusValid()
        {
            SetRxStatus(EndpointStatus::Valid);
        }

        /**
         * @brief Clear all interrupts for endpoint
         * 
         * @par Returns
         *  Nothing
         */
        static void ClearAllRxInterrupts()
        {
            _Regs()->DOEPINT = _Regs()->DOEPINT;
        }

        /**
         * @brief RX fifo not empty handler
         * 
         * @param size Received size
         * 
         * @par Returns
         *  Nothing
         */
        static void HandlerFifoNotEmpty(uint16_t size)
        {
            for(int i = 0; i < (size + 3) / 4; ++i)
            {
                reinterpret_cast<uint32_t*>(Buffer + BufferSize)[i] = *reinterpret_cast<volatile uint32_t*>(_FifoAddress);
            }
            BufferSize += size;
        }

        /**
         * @brief Set out data transfer callback (RX handler can try call this method if cannot handle packet)
         * 
         * @param [in] callback Callback
         * 
         * @par Returns
         *  Nothing
         */
        static void SetOutDataTransferCallback(OutTransferCallback callback)
        {
            _dataTransferCallback = callback;
        }

        /**
         * @brief Reset out data transfer callback
         * 
         * @par Returns
         *  Nothing
         */
        static void ResetOutDataTransferCallback()
        {
            _dataTransferCallback = nullptr;
        }

        /**
         * @brief Try call transfer callback
         * 
         * @par Returns
         *  Nothing
         */
        static void TryHandleDataTransfer()
        {
            if (_dataTransferCallback != nullptr)
                _dataTransferCallback();
        }

    private:
        static OutTransferCallback _dataTransferCallback;
    };


    template<typename _Base, typename _Regs, uint32_t _FifoAddress>
    OutTransferCallback OutEndpoint<_Base, _Regs, _FifoAddress>::_dataTransferCallback = nullptr;

    template<typename _Base, typename _Regs, uint32_t _FifoAddress>
    uint8_t OutEndpoint<_Base, _Regs, _FifoAddress>::BufferSize = 0;

    template<typename _Base, typename _Regs, uint32_t _FifoAddress>
    uint8_t OutEndpoint<_Base, _Regs, _FifoAddress>::Buffer[_Base::MaxPacketSize] = {};

    /**
     * @brief Implements in (TX) endpoint
     * 
     * @tparam _Base Endpoint base
     * @tparam _Regs EP registers wrapper
     * @tparam _FifoNumber TX FIFO number
     * @tparam _BufferAddress TX buffer address
     */
    template<typename _Base, typename _Regs, uint8_t _FifoNumber, uint32_t _FifoAddress>
    class InEndpoint : public Endpoint<_Base>
    {
        using Base = Endpoint<_Base>;
        static const bool SendZlp = !(requires {_Base::DisableZlp;}); 
        
        static const uint32_t DiepEmpMskAddress = USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE + offsetof(USB_OTG_DeviceTypeDef, DIEPEMPMSK);
        using TxFifoEmptyInterruptMask = IoBit<DiepEmpMskAddress, uint32_t, Base::Number>;
    public:

        /**
         * @brief Reset endpoint
         */
        static void Reset()
        {
            if constexpr (Base::Number == 0)
            {
                static_assert(Base::MaxPacketSize == 8 || Base::MaxPacketSize == 16 || Base::MaxPacketSize == 32 || Base::MaxPacketSize == 64);

                _Regs()->DIEPCTL = USB_OTG_DIEPCTL_SNAK
                    | (_FifoNumber << USB_OTG_DIEPCTL_TXFNUM_Pos)
                    | USB_OTG_DIEPCTL_USBAEP
                    | CalculateMpsizValueForZeroEndpoint(Base::MaxPacketSize);
            }
            else
            {
                _Regs()->DIEPCTL = USB_OTG_DIEPCTL_SNAK
                    | (_FifoNumber << USB_OTG_DIEPCTL_TXFNUM_Pos)
                    | (static_cast<uint32_t>(Base::Type) << USB_OTG_DIEPCTL_EPTYP_Pos)
                    | USB_OTG_DIEPCTL_USBAEP
                    | Base::MaxPacketSize;
            }
        }
        
        /**
         * @brief Endpoint event handled (TX fifo empty or transfer complete)
         * 
         * @par Returns
         *  Nothing
         */
        static inline void Handler()
        {
            HandleTx();
        }

        /**
         * @brief IN endpoint Handler
         * 
         * @par Returns
         *  Nothing
         */
        static void HandleTx()
        {
            if (_Regs()->DIEPINT & USB_OTG_DIEPINT_XFRC) {
                _Regs()->DIEPINT = USB_OTG_DIEPINT_XFRC;
                HandleTxComplete();
                return;
            }

            if (_Regs()->DIEPINT & USB_OTG_DIEPINT_TXFE) {
                HandleFifoEmpty();
                return;
            }
        }

        /**
         * @brief Set endpoint`s TX status
         *
         * @param [in] status New status
         *
         * @par Returns
         *  Nothing
        */
        static void SetTxStatus(EndpointStatus status)
        {
            switch (status)
            {
            case EndpointStatus::Disable:
                if (_Regs()->DIEPCTL & USB_OTG_DIEPCTL_EPENA) {
                    _Regs()->DIEPCTL |= USB_OTG_DIEPCTL_EPDIS;
                }
                break;
            case EndpointStatus::Stall:
                _Regs()->DIEPCTL |= USB_OTG_DIEPCTL_STALL;
                break;
            case EndpointStatus::Nak:
                _Regs()->DIEPCTL |= USB_OTG_DIEPCTL_SNAK;
                break;
            case EndpointStatus::Valid:
                _Regs()->DIEPCTL |= USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA;
                break;
            }
        }

        /**
         * @brief Clear all endpoint`s interrupts
         * 
         * @par Returns
         *  Nothing
         */
        static void ClearAllTxInterrupts()
        {
            _Regs()->DIEPINT = _Regs()->DIEPINT;
        }

        /**
         * @brief Send ZLP to host
         * 
         * @param [in, opt] callback Complete callback
         */
        static void SendZLP(InTransferCallback callback = nullptr)
        {
            _needZlpSend = false;
            _txCompleteCallback = callback;
            _Regs()->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos)
                | (0 << USB_OTG_DIEPTSIZ_XFRSIZ_Pos);
            SetTxStatus(EndpointStatus::Valid);
        }

        /**
         * @brief Send data
         * 
         * @param [in] data Data
         * @param [in] size Data size
         * @param [in, opt] callback Complete callback
         */
        static void SendData(const void* data, uint32_t size, InTransferCallback callback = nullptr)
        {
            _dataToTransmit = reinterpret_cast<const uint32_t*>(data);
            _needZlpSend = size % _Base::MaxPacketSize == 0;
            _txCompleteCallback = callback;

            uint32_t packetsCount = (size + _Base::MaxPacketSize - 1) / _Base::MaxPacketSize;

            _Regs()->DIEPTSIZ = (packetsCount << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | (size << USB_OTG_DIEPTSIZ_XFRSIZ_Pos);

            TxFifoEmptyInterruptMask::Set();

            SetTxStatus(EndpointStatus::Valid);
        }
    protected:
        /**
         * @brief TX complete handler
         * 
         * @par Returns
         *  Nothing
         */
        static void HandleTxComplete()
        {
            TxFifoEmptyInterruptMask::Clear();

            if constexpr (SendZlp) {
                if (_needZlpSend) {
                    SendZLP(_txCompleteCallback);
                    return;
                }
            }

            if(_txCompleteCallback)
                _txCompleteCallback();
        }
        
        /**
         * @brief Handle TX FIFO not empty
         * 
         * @par Returns
         *  Nothing
         */
        static void HandleFifoEmpty()
        {
            SendPacket();
        }

        /**
         * @brief Send packet
         * 
         * @param size Packet size
         * 
         * @par Returns
         *  Nothing
         */
        inline static void SendPacket()
        {
            int32_t bytesRemain = _Regs()->DIEPTSIZ & USB_OTG_DIEPTSIZ_XFRSIZ;

            int32_t doubleWordsToWrite = (
                (bytesRemain < _Base::MaxPacketSize
                    ? bytesRemain
                    : _Base::MaxPacketSize)
                + sizeof(uint32_t) - 1) / sizeof(uint32_t);

            for(int i = 0; i < doubleWordsToWrite; ++i) {
                *reinterpret_cast<volatile uint32_t*>(_FifoAddress) = *(_dataToTransmit++);
            }
        }

    private:
        static const uint32_t* _dataToTransmit;
        static bool _needZlpSend;
        static InTransferCallback _txCompleteCallback;
    };
    
    template<typename _Base, typename _Regs, uint8_t _FifoNumber, uint32_t _FifoAddress>
    const uint32_t* InEndpoint<_Base, _Regs, _FifoNumber, _FifoAddress>::_dataToTransmit = nullptr;

    template<typename _Base, typename _Regs, uint8_t _FifoNumber, uint32_t _FifoAddress>
    bool InEndpoint<_Base, _Regs, _FifoNumber, _FifoAddress>::_needZlpSend = false;

    template<typename _Base, typename _Regs, uint8_t _FifoNumber, uint32_t _FifoAddress>
    InTransferCallback InEndpoint<_Base, _Regs, _FifoNumber, _FifoAddress>::_txCompleteCallback = nullptr;

    /**
     * @brief Implements bidirectional endpoint
     * 
     * @tparam _Base Base endpoint
     * @tparam _InReg IN register
     * @tparam _OutReg OUT register
     * @tparam _FifoNumber FIFO number
     * @tparam _FifoAddress FIFO address
     */
    template<typename _Base, typename _InReg, typename _OutReg, uint8_t _FifoNumber, uint32_t _FifoAddress>
    class BidirectionalEndpoint : public OutEndpoint<_Base, _OutReg, _FifoAddress>, public InEndpoint<_Base, _InReg, _FifoNumber, _FifoAddress>
    {
        using Base = _Base;
        using In = InEndpoint<_Base, _InReg, _FifoNumber, _FifoAddress>;
        using Out = OutEndpoint<_Base, _OutReg, _FifoAddress>;
    public:
        using InRegs = _InReg;

        static const uint16_t Number = _Base::Number;
        static const EndpointDirection Direction = _Base::Direction;
        static const EndpointType Type = _Base::Type;
        static const uint16_t MaxPacketSize = _Base::MaxPacketSize;
        static const uint8_t Interval = _Base::Interval;

        static constexpr uint8_t* RxBuffer = Out::Buffer;
        
        static void Reset()
        {
            In::Reset();
            Out::Reset();
        }

        static void Handler()
        {
            In::HandleTx();
            if(_OutReg()->DOEPINT & USB_OTG_DOEPINT_XFRC)
            {
                _OutReg()->DOEPINT = USB_OTG_DOEPINT_XFRC;
                HandleRx();
            }
        }

        /**
         * @brief Send ZLP
         * 
         * @param callback Transmit complete callback (Set RX valid as default)
         * 
         * @par Returns
         *  Nothing
         */
        inline static void SendZLP(InTransferCallback callback = Out::SetRxStatusValid)
        {
            In::SendZLP(callback);
        }

        /**
         * @brief Send data
         * 
         * @param data Data to transmit
         * @param size Data size
         * @param callback Transmit complete callback (Set RX valid as default)
         */
        inline static void SendData(const void* data, uint32_t size, InTransferCallback callback = Out::SetRxStatusValid)
        {
            In::SendData(data, size, callback);
        }

        template<typename T = uint32_t>
        static std::enable_if_t<_Base::Number == 0, T> GetOutInterrupts()
        {
            return _OutReg()->DOEPINT;
        }

        template<typename T = uint32_t>
        static std::enable_if_t<_Base::Number == 0, T> GetInInterrupts()
        {
            return _InReg()->DIEPINT;
        }

        static void HandleRx();
    };
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_OTG_ENDPOINT_H
