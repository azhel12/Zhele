/**
 * @file
 * Implement USB endpoint
 * 
 * @author Aleksei Zhelonkin
 * Endpoint`s registers bits manipulation code is taken from libopencm3 project.
 * Original source: https://github.com/libopencm3/libopencm3/blob/master/include/libopencm3/stm32/common/st_usbfs_common.h
 * Original author: Piotr Esden-Tempski <piotr@esden.net>. Thanks him very much.
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_ENDPOINT_H
#define ZHELE_USB_ENDPOINT_H

#include "../template_utils/type_list.h"
#include "../template_utils/static_array.h"

#include "common.h"

#include <functional>

namespace Zhele::Usb
{
    /**
     * @brief Endpoint type values for EPnR registers.
     */
    using EndpointsTypesForEPR = Zhele::UnsignedArray<USB_EP_CONTROL, USB_EP_ISOCHRONOUS, USB_EP_BULK, USB_EP_INTERRUPT, USB_EP_CONTROL | USB_EP_KIND, 0, USB_EP_BULK | USB_EP_KIND>;
    /**
     * @brief Endpoint (transfer) type
     */
    enum class EndpointType : uint8_t
    {
        Control = 0,
        Isochronous = 1,
        Bulk = 2,
        Interrupt = 3,
        // Additional types
        ControlStatusOut = 4,
        BulkDoubleBuffered = 6
    };
    /**
     * @brief Endpoint direction
     */
    enum class EndpointDirection : uint8_t
    {
        Out = 0, //< Out
        In = 1, //< In
        Bidirectional = 2, //< Bidirectional endpoint. On USB layer Will be split on two endpoints.
    };
    /**
     * @brief Endpoint statis (RX or TX)
     */
    enum class EndpointStatus : uint8_t
    {
        Disable = 0, //< Disabled
        Stall = 1, //< Stall
        Nak = 2, //< Nak
        Valid = 3, //< Valid
    };

    /**
     * @brief Endpoint descriptor
     */
    #pragma pack(push, 1)
    struct EndpointDescriptor
    {
        uint8_t Length = 7;
        DescriptorType Type = DescriptorType::Endpoint;
        uint8_t Address;
        uint8_t Attributes;
        uint16_t MaxPacketSize;
        uint8_t Interval;
    };
    #pragma pack(pop)

    /**
     * @brief Endpoint`s packet buffer descriptor.
     */
    struct PacketBufferDescriptor
    {
        uint16_t TxAddress;
        uint16_t TxCount;
        uint16_t RxAddress;
        uint16_t RxCount;
    };

    /**
     * @brief Implements endpoint base
     * 
     * @tparam _Number Endpoint number (address)
     * @tparam _Direction Endpoint direction
     * @tparam _Type Endpoint type
     * @tparam _MaxPacketSize Max packet size
     * @tparam _Interval Polling interval
     */
    template <uint8_t _Number, EndpointDirection _Direction, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval = 0>
    class EndpointBase
    {
    public:
        static const uint16_t Number = _Number;
        static const EndpointDirection Direction = _Direction;
        static const EndpointType Type = _Type;
        static const uint16_t MaxPacketSize = _MaxPacketSize;
        static const uint8_t Interval = _Interval;
    };

    template<uint8_t _Number, EndpointDirection _Direction, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class UniDirectionalEndpointBase : public EndpointBase<_Number, _Direction, _Type, _MaxPacketSize, _Interval>
    {
    };

    template<uint8_t _Number, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class OutEndpointBase : public UniDirectionalEndpointBase<_Number, EndpointDirection::Out, _Type, _MaxPacketSize, _Interval>
    {
    };

    template<uint8_t _Number, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class InEndpointBase : public UniDirectionalEndpointBase<_Number, EndpointDirection::In, _Type, _MaxPacketSize, _Interval>
    {
    };

    template<uint8_t _Number, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class BidirectionalEndpointBase : public EndpointBase<_Number, EndpointDirection::Bidirectional, _Type, _MaxPacketSize, _Interval>
    {
    };

    template<uint8_t _Number, uint16_t _MaxPacketSize, uint8_t _Interval>
    class BulkDoubleBufferedEndpointBase : public EndpointBase<_Number, EndpointDirection::Bidirectional, EndpointType::BulkDoubleBuffered, _MaxPacketSize, _Interval>
    {
    };

    template <uint8_t _Number, uint16_t _MaxPacketSize>
    class ControlEndpointBase : public EndpointBase<_Number, EndpointDirection::Bidirectional, EndpointType::Control, _MaxPacketSize, 0>
    {      
    };

    template <uint16_t _MaxPacketSize>
    class ZeroEndpointBase : public ControlEndpointBase<0, _MaxPacketSize>
    {
    }; 

    /**
     * @brief Implements endpoint
     * 
     * @tparam _Base Base endpoint (EndpointBase specialization)
     * @tparam _Reg EPnR register
     */
    template <typename _Base, typename _Reg>
    class Endpoint : public _Base
    {
    public:        
        static const uint16_t Number = _Base::Number;
        static const EndpointDirection Direction = _Base::Direction;
        static const EndpointType Type = _Base::Type;
        static const uint16_t MaxPacketSize = _Base::MaxPacketSize;
        static const uint8_t Interval = _Base::Interval;

        using Reg = _Reg;

        /**
         * @brief Reset endpoint
         * 
         * @par Returns
         *  Nothing
         */         
        static void Reset()
        {
            Reg::Set((Number & 0x0f)
                | static_cast<uint16_t>(GetNumber<static_cast<int>(Type), EndpointsTypesForEPR>::value));

            SetRxStatus(EndpointStatus::Valid);
            SetTxStatus(EndpointStatus::Nak);
        }

        /**
         * @brief Fills descriptor
         * 
         * @param [out] descriptor Pointer to fill.
         *
         * @par Returns
         *  Nothing
         */
        static uint16_t FillDescriptor(EndpointDescriptor* descriptor)
        {
            *descriptor = EndpointDescriptor{
                .Address = static_cast<uint8_t>(Number) | ((static_cast<uint8_t>(Direction) & 0x01) << 7),
                .Attributes = static_cast<uint8_t>(Type) & 0x3,
                .MaxPacketSize = MaxPacketSize,
                .Interval = Interval};

            if constexpr(Direction == EndpointDirection::Bidirectional)
            {
                ++descriptor;
                *descriptor = EndpointDescriptor{
                    .Address = static_cast<uint8_t>(Number) | (1 << 7),
                    .Attributes = static_cast<uint8_t>(Type),
                    .MaxPacketSize = MaxPacketSize,
                    .Interval = Interval};

                return 2 * sizeof(EndpointDescriptor);
            }

            return sizeof(EndpointDescriptor);
        }

        /**
         * @brief Set endpoint`s RX status.
         *
         * @param [in] status New status.
         *
         * @par Returns
         *  Nothing
        */
        static void SetRxStatus(EndpointStatus status)
        {
            ToogleAndSet<USB_EPREG_MASK | USB_EPRX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<uint16_t>(status) << 12);
        }
        /**
         * @brief Set endpoint`s TX status.
         *
         * @param [in] status New status.
         *
         * @par Returns
         *  Nothing
        */
        static void SetTxStatus(EndpointStatus status)
        {
            ToogleAndSet<USB_EPREG_MASK | USB_EPTX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<uint16_t>(status) << 4);
        }
        /**
         * @brief Clear endpoint`s RX flag.
         *
         * @par Returns
         *  Nothing
        */
        static void ClearCtrRx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_TX>(USB_EP_CTR_RX);
        }
        /**
         * @brief Clear endpoint`s TX flag.
         *
         * @par Returns
         *  Nothing
        */
        static void ClearCtrTx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_RX>(USB_EP_CTR_TX);
        }
        /**
         * @brief Clear endpoint`s TX_DTOG flag.
         *
         * @par Returns
         *  Nothing
        */
        static void ClearTxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_TX);
        }
        /**
         * @brief Clear endpoint`s RX_DTOG flag.
         *
         * @par Returns
         *  Nothing
        */
        static void ClearRxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_RX);
        }
        /**
         * @brief Endpoint interrupt handler.
         *
         * @par Returns
         *  Nothing
        */
        static void Handler();
    private:
        template<uint16_t Mask, uint16_t ExtraBits = 0>
        static void ToogleAndSet(uint16_t Bit)
        {
            uint16_t toggleMask = Reg::Get() & Mask;
            toggleMask ^= Bit;
            Reg::Set(toggleMask | ExtraBits);
        }

        template<uint16_t Mask, uint16_t ExtraBits = 0>
        static void ClearRegBitMaskAndSet(uint16_t Bit)
        {
            uint16_t clrMask = Reg::Get() & Mask & ~Bit;
            Reg::Set(clrMask | ExtraBits);
        }
    };

    template<typename _Endpoint, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class EndpointWriter
    {
        using BufferCountReg = RegisterWrapper<_CountRegAddress, uint16_t>;
    public:
        static void SendData(uint16_t size)
        {
            BufferCountReg::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }

        static void SendData(const void* data, uint16_t size)
        {
            const uint16_t* source = reinterpret_cast<const uint16_t*>(data);
            uint16_t* destination = reinterpret_cast<uint16_t*>(_BufferAddress);
            for(uint16_t i = 0; i < (size + 1) / 2; ++i)
                destination[PmaAlignMultiplier * i] = source[i];

            BufferCountReg::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }
    };

    // Using std::function allows lambdas as callbacks, but takes ~1,2Kb flash and ~100 bytes RAM.
    // Please advise me best solution.
    using InTransferCallback = std::function<void()>;
    //using InTransferCallback = std::add_pointer_t<void()>;
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class EndpointWithTxSupport
    {
        using Ep = Endpoint<_Base, _Reg>;
        using Writer = EndpointWriter<Ep, _BufferAddress, _CountRegAddress>;
    public:
        static void SendZLP(InTransferCallback callback = SetEpRxStatusValid)
        {
            _txCompleteCallback = callback;
            Writer::SendData(0);
        }

        static void SendData(const void* data, uint32_t size, InTransferCallback callback = SetEpRxStatusValid)
        {
            _dataToTransmit = reinterpret_cast<const uint8_t*>(data);
            _bytesRemain = size;
            _txCompleteCallback = callback;
            Writer::SendData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
        }
    protected:
        static void HandleTx()
        {
            _bytesRemain -= _Base::MaxPacketSize;
            _dataToTransmit += _Base::MaxPacketSize;

            if(_bytesRemain >= 0)
            {
                Writer::SendData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
                return;
            }
            if(_txCompleteCallback)
            {
                _txCompleteCallback();
            }
        }
    private:
        static void SetEpRxStatusValid()
        {
            Ep::SetRxStatus(EndpointStatus::Valid);
        }

        static const uint8_t* _dataToTransmit;
        static int32_t _bytesRemain;
        static InTransferCallback _txCompleteCallback;
    };
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    const uint8_t* EndpointWithTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>::_dataToTransmit = nullptr;
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    int32_t EndpointWithTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>::_bytesRemain = -1;
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    InTransferCallback EndpointWithTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>::_txCompleteCallback = nullptr;

    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class OutEndpoint : public Endpoint<_Base, _Reg>
    {
        using Base = Endpoint<_Base, _Reg>;
    public:
        static constexpr uint32_t Buffer = _BufferAddress;
        using BufferCount = RegisterWrapper<_CountRegAddress, uint16_t>;
        static void Handler()
        {
            Base::ClearCtrRx();
            HandleRx();       
        }

        static void HandleRx();
    };

    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class InEndpoint : public Endpoint<_Base, _Reg>, public EndpointWithTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>
    {
        using Base = Endpoint<_Base, _Reg>;
        using TxModule = EndpointWithTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>;
    public:
        static constexpr uint32_t Buffer = _BufferAddress;
        using BufferCount = RegisterWrapper<_CountRegAddress, uint16_t>;
        static void Handler()
        {
            Base::ClearCtrTx();
            TxModule::HandleTx();
        }
    };

    template<typename _Base, typename _Reg, uint32_t _TxBufferAddress, uint32_t _TxCountRegAddress, uint32_t _RxBufferAddress, uint32_t _RxCountRegAddress>
    class BidirectionalEndpoint : public Endpoint<_Base, _Reg>, public EndpointWithTxSupport<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress>
    {
        using This = BidirectionalEndpoint<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress, _RxBufferAddress, _RxCountRegAddress>;
        using Base = Endpoint<_Base, _Reg>;
        using TxModule = EndpointWithTxSupport<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress>;
    public:
        using Reg = _Reg;
        static constexpr uint32_t TxBuffer = _TxBufferAddress;
        using TxBufferCount = RegisterWrapper<_TxCountRegAddress, uint16_t>;
        static constexpr uint32_t RxBuffer = _RxBufferAddress;
        using RxBufferCount = RegisterWrapper<_RxCountRegAddress, uint16_t>;

        static void Handler()
        {
            if(Reg::Get() & USB_EP_CTR_RX)
            {
                Base::ClearCtrRx();
                HandleRx();
            }
            if(Reg::Get() & USB_EP_CTR_TX)
            {
                Base::ClearCtrTx();
                TxModule::HandleTx();
            }
        }

        static void HandleRx();
    };

    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    class BulkDoubleBufferedEndpoint : public Endpoint<_Base, _Reg>
    {
        using This = BidirectionalEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address,_Count1RegAddress>;
    public:
        using Reg = _Reg;
        static constexpr void* Buffer0 = _Buffer0Address;
        using Buffer0Count = RegisterWrapper<_Count0RegAddress, uint16_t>;
        static constexpr void* Buffer1 = _Buffer1Address;
        using Buffer1Count = RegisterWrapper<_Count1RegAddress, uint16_t>;
    };

    using DefaultEp0 = ZeroEndpointBase<16>;
}
#endif // ZHELE_USB_ENDPOINT_H