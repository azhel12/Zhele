/**
 * @file
 * Implement platform-independent part of USB endpoint
 *
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_COMMON_USB_ENDPOINT_H
#define ZHELE_COMMON_USB_ENDPOINT_H

#include "common.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace Zhele::Usb
{
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
        Out = 0, ///< Out
        In = 1, ///< In
        Bidirectional = 2, ///< Bidirectional endpoint. On USB layer Will be split on two endpoints.
    };
    /**
     * @brief Endpoint statis (RX or TX)
     */
    enum class EndpointStatus : uint8_t
    {
        Disable = 0, ///< Disabled
        Stall = 1, ///< Stall
        Nak = 2, ///< Nak
        Valid = 3, ///< Valid
    };

    /**
     * @brief Endpoint descriptor
     */
    #pragma pack(push, 1)
    struct EndpointDescriptor
    {
        uint8_t Length = 7; ///< Length (always 7)
        DescriptorType Type = DescriptorType::Endpoint; ///< Descriptor type (always 0x05)
        uint8_t Address; ///< Endpoint address
        uint8_t Attributes; ///< Endpoint attributes
        uint16_t MaxPacketSize; ///< Endpoint max packet size
        uint8_t Interval; ///< Endpoint poll interval

        /**
         * @brief Return descriptor bytes
         *
         * @returns Bytes of descriptor
        */
        constexpr auto GetBytes() const
        {
            return std::array<uint8_t, 7> {
                Length,
                static_cast<uint8_t>(Type),
                Address,
                Attributes,
                static_cast<uint8_t>(MaxPacketSize & 0xff), static_cast<uint8_t>((MaxPacketSize >> 8) & 0xff),
                Interval
            };
        }
    };
    #pragma pack(pop)

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

        /**
         * @brief Build endpoint descriptor
         *
         * @returns Bytes of descriptor
         */
        static consteval auto GetDescriptor()
        {
            constexpr unsigned size = Direction == EndpointDirection::Bidirectional
                ? 2 * sizeof(EndpointDescriptor)
                : sizeof(EndpointDescriptor);

            std::array<uint8_t, size> result;

            constexpr auto desc = EndpointDescriptor{
                .Address = static_cast<uint8_t>(Number) | ((static_cast<uint8_t>(Direction) & 0x01) << 7),
                .Attributes = static_cast<uint8_t>(Type) & 0x3,
                .MaxPacketSize = MaxPacketSize,
                .Interval = Interval}.GetBytes();
            auto dst = std::copy(desc.begin(), desc.end(), result.begin());

            if constexpr(Direction == EndpointDirection::Bidirectional)
            {
                constexpr auto secondDesc = EndpointDescriptor{
                    .Address = static_cast<uint8_t>(Number) | (1 << 7),
                    .Attributes = static_cast<uint8_t>(Type),
                    .MaxPacketSize = MaxPacketSize,
                    .Interval = Interval}.GetBytes();
                std::copy(secondDesc.begin(), secondDesc.end(), dst);
            }

            return result;
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
    class InEndpointWithoutZlpBase : public UniDirectionalEndpointBase<_Number, EndpointDirection::In, _Type, _MaxPacketSize, _Interval>
    {
    public:
        const static bool DisableZlp;
    };

    template<uint8_t _Number, EndpointType _Type, uint16_t _MaxPacketSize, uint8_t _Interval>
    class BidirectionalEndpointBase : public EndpointBase<_Number, EndpointDirection::Bidirectional, _Type, _MaxPacketSize, _Interval>
    {
    };

    template<uint8_t _Number, EndpointDirection _Direction, uint16_t _MaxPacketSize>
    class BulkDoubleBufferedEndpointBase : public EndpointBase<_Number, _Direction, EndpointType::BulkDoubleBuffered, _MaxPacketSize, 0>
    {
    };

    template<uint8_t _Number, uint16_t _MaxPacketSize>
    class InBulkDoubleBufferedWithoutZlpEndpointBase : public EndpointBase<_Number, EndpointDirection::In, EndpointType::BulkDoubleBuffered, _MaxPacketSize, 0>
    {
    public:
        const static bool DisableZlp;
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
     * @brief Default Ep0 instance
     */
    using DefaultEp0 = ZeroEndpointBase<64>;

    // Using std::function allows lambdas as callbacks, but takes ~1,2Kb flash and ~100 bytes RAM
    // Please advise me best solution
    //using InTransferCallback = std::function<void()>;
    using InTransferCallback = std::add_pointer_t<void()>;
    using OutTransferCallback = std::add_pointer_t<void()>;

    /**
     * @brief Endpoint with TX feature
     *
     * @details Splits data into packets of MaxPacketSize bytes. Writing a single packet is
     * platform-specific and is done by _Writer::SendData(data, size)
     *
     * @tparam _Base Endpoint base
     * @tparam _Ep Endpoint (for SetRxStatus)
     * @tparam _Writer Packet writer
     */
    template<typename _Base, typename _Ep, typename _Writer>
    class EndpointWithTxSupport
    {
    public:
        /**
         * @brief Send ZLP to host
         *
         * @param [in, opt] callback Complete callback
         */
        static void SendZLP(InTransferCallback callback = SetEpRxStatusValid)
        {
            _bytesRemain = 0;
            _txCompleteCallback = callback;
            _Writer::SendData(0);
        }

        /**
         * @brief Send data
         *
         * @param [in] data Data
         * @param [in] size Data size
         * @param [in, opt] callback Complete callback
         */
        static void SendData(const void* data, uint32_t size, InTransferCallback callback = SetEpRxStatusValid)
        {
            _dataToTransmit = reinterpret_cast<const uint8_t*>(data);
            _bytesRemain = size;
            _txCompleteCallback = callback;
            _Writer::SendData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
        }

        /**
         * @brief TX complete handler (sends next packet or calls complete callback)
         * 
         * @par Returns
         *  Nothing
         */
        static void HandleTx()
        {
            _bytesRemain -= _Base::MaxPacketSize;
            _dataToTransmit += _Base::MaxPacketSize;

            if(_bytesRemain >= 0)
            {
                _Writer::SendData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
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
            _Ep::SetRxStatus(EndpointStatus::Valid);
        }

        static const uint8_t* _dataToTransmit;
        static int32_t _bytesRemain;
        static InTransferCallback _txCompleteCallback;
    };

    template<typename _Base, typename _Ep, typename _Writer>
    const uint8_t* EndpointWithTxSupport<_Base, _Ep, _Writer>::_dataToTransmit = nullptr;
    template<typename _Base, typename _Ep, typename _Writer>
    int32_t EndpointWithTxSupport<_Base, _Ep, _Writer>::_bytesRemain = -1;
    template<typename _Base, typename _Ep, typename _Writer>
    InTransferCallback EndpointWithTxSupport<_Base, _Ep, _Writer>::_txCompleteCallback = nullptr;

    /**
     * @brief Endpoint with RX feature
     */
    class EndpointWithRxSupport
    {
    public:
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
         * @brief Try call callback if it exists
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
        static inline OutTransferCallback _dataTransferCallback = nullptr;
    };
}
#endif // ZHELE_COMMON_USB_ENDPOINT_H
