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
     * @brief Implements endpoint
     * 
     * @tparam _Reg EPnR register (full). Please, be careful in unidirectional endpoints.
     * @tparam _Number Endpoint number (address)
     * @tparam _Direction Endpoint direction
     * @tparam _Type Endpoint type
     * @tparam _MaxPacketSize Max packet size
     * @tparam _Interval Polling interval
     */
    template <uint8_t _Number, EndpointDirection _Direction, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class EndpointBase
    {
    public:
        static const uint16_t Number = _Number;
        static const EndpointDirection Direction = _Direction;
        static const EndpointType Type = _Type;
        static const uint16_t MaxPacketSize = _MaxPacketSize;
        static const uint8_t Interval = _Interval;
    };

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

        static void Reset()
        {
            Reg::Set((Number & 0x0f)
                | static_cast<uint16_t>(GetNumber<static_cast<int>(Type), EndpointsTypesForEPR>::value));

            SetRxStatus(EndpointStatus::Valid);
            SetTxStatus(EndpointStatus::Nak);
        }

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

        static void SetRxStatus(EndpointStatus status)
        {
            ToogleAndSet<USB_EPREG_MASK | USB_EPRX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<uint16_t>(status) << 12);
        }
        static void SetTxStatus(EndpointStatus status)
        {
            ToogleAndSet<USB_EPREG_MASK | USB_EPTX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<uint16_t>(status) << 4);
        }
        static void ClearCtrRx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_TX>(USB_EP_CTR_RX);
        }
        static void ClearCtrTx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_RX>(USB_EP_CTR_TX);
        }

        static void ClearTxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_TX);
        }
        static void ClearRxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_RX);
        }

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

    template<typename _Endpoint>
    class EndpointWriter
    {
    public:
        static void SendData(uint16_t size)
        {
            _Endpoint::TxBufferCount::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }

        static void SendData(const void* data, uint16_t size)
        {
            const uint16_t* source = reinterpret_cast<const uint16_t*>(data);
            uint16_t* destination = reinterpret_cast<uint16_t*>(_Endpoint::TxBuffer);
            for(uint16_t i = 0; i < (size + 1) / 2; ++i)
                destination[PmaAlignMultiplier * i] = source[i];

            _Endpoint::TxBufferCount::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }
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

    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class UniDirectionalEndpoint : public Endpoint<_Base, _Reg>
    {
    public:
        static constexpr uint32_t Buffer = _BufferAddress;
        using BufferCount = RegisterWrapper<_CountRegAddress, uint16_t>;
        static void Handler();
    };

    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    using OutEndpoint = UniDirectionalEndpoint<_Base, _Reg, _BufferAddress, _CountRegAddress>;

    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    using InEndpoint = UniDirectionalEndpoint<_Base, _Reg, _BufferAddress, _CountRegAddress>;

    template<typename _Base, typename _Reg, uint32_t _TxBufferAddress, uint32_t _TxCountRegAddress, uint32_t _RxBufferAddress, uint32_t _RxCountRegAddress>
    class BidirectionalEndpoint : public Endpoint<_Base, _Reg>
    {
        using This = BidirectionalEndpoint<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress, _RxBufferAddress, _RxCountRegAddress>;
    public:
        using Reg = _Reg;
        static constexpr uint32_t TxBuffer = _TxBufferAddress;
        using TxBufferCount = RegisterWrapper<_TxCountRegAddress, uint16_t>;
        static constexpr uint32_t RxBuffer = _RxBufferAddress;
        using RxBufferCount = RegisterWrapper<_RxCountRegAddress, uint16_t>;
        using Writer = EndpointWriter<This>;

        static void Handler();
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
        using Writer = EndpointWriter<This>;
    };

    template<typename _Base, typename _Reg, uint32_t _TxBufferAddress, uint32_t _TxCountRegAddress, uint32_t _RxBufferAddress, uint32_t _RxCountRegAddress>
    class ControlEndpoint : public Endpoint<_Base, _Reg>
    {
        using This = ControlEndpoint<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress, _RxBufferAddress,_RxCountRegAddress>;
    public:
        using Reg = _Reg;
        static const uint32_t TxBuffer = _TxBufferAddress;
        using TxBufferCount = RegisterWrapper<_TxCountRegAddress, uint16_t>;
        static const uint32_t RxBuffer = _RxBufferAddress;
        using RxBufferCount = RegisterWrapper<_RxCountRegAddress, uint16_t>;

        using Writer = EndpointWriter<This>;
    };

    using DefaultEp0 = ZeroEndpointBase<64>;
}
#endif // ZHELE_USB_ENDPOINT_H