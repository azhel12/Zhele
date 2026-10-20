/**
 * @file
 * Implement USB endpoint for PMA-based device controller (incl. USB_DRD_FS)
 * 
 * @author Aleksei Zhelonkin
 * 
 * Endpoint`s registers bits manipulation code is taken from libopencm3 project
 * Original source: https://github.com/libopencm3/libopencm3/blob/master/include/libopencm3/stm32/common/st_usbfs_common.h
 * Original author: Piotr Esden-Tempski <piotr@esden.net>. Thanks him very much
 * 
 * For help with improvement of double-buffered endpoints big thanks to Pavel Loktev (aka @EasyLy on habr.com)
 * 
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINT_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINT_H

#include <zhele/common/usb/endpoint.h>

#include "../hw.h"

#include <array>
#include <cstddef>

namespace Zhele::Usb
{
    /**
     * @brief Endpoint type values for EPnR registers (indexed by EndpointType)
     */
    inline constexpr std::array<EndpointRegisterType, 7> endpoints_types_for_epr{
        USB_EP_CONTROL,
        USB_EP_ISOCHRONOUS,
        USB_EP_BULK,
        USB_EP_INTERRUPT,
        static_cast<EndpointRegisterType>(USB_EP_CONTROL | USB_EP_KIND),
        0,
        static_cast<EndpointRegisterType>(USB_EP_BULK | USB_EP_KIND)};

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
                | endpoints_types_for_epr[static_cast<std::size_t>(Type)]);

            if constexpr (Direction != EndpointDirection::In)
            {
                SetRxStatus(EndpointStatus::Valid);
            }
            SetTxStatus(EndpointStatus::Nak);
        }

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
            ToogleAndSet<USB_EPREG_MASK | USB_EPRX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<EndpointRegisterType>(status) << 12);
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
            ToogleAndSet<USB_EPREG_MASK | USB_EPTX_STAT, USB_EP_CTR_TX | USB_EP_CTR_RX>(static_cast<EndpointRegisterType>(status) << 4);
        }
        /**
         * @brief Clear endpoint`s RX flag
         *
         * @par Returns
         *  Nothing
        */
        static void ClearCtrRx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_TX>(USB_EP_CTR_RX);
        }
        /**
         * @brief Clear endpoint`s TX flag
         *
         * @par Returns
         *  Nothing
        */
        static void ClearCtrTx()
        {
            ClearRegBitMaskAndSet<USB_EPREG_MASK, USB_EP_CTR_RX>(USB_EP_CTR_TX);
        }
        /**
         * @brief Clear endpoint`s TX_DTOG flag
         *
         * @par Returns
         *  Nothing
        */
        static void ClearTxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_TX);
        }

        /**
         * @brief Set endpoint`s TX_DTOG flag
         *
         * @par Returns
         *  Nothing
        */
        static void SetTxDtog()
        {
            Toggle<USB_EP_DTOG_TX, USB_EP_CTR_RX | USB_EP_CTR_TX>();
        }

        /**
         * @brief Clear endpoint`s RX_DTOG flag
         *
         * @par Returns
         *  Nothing
        */
        static void ClearRxDtog()
        {
            Reg::And(USB_EPREG_MASK | USB_EP_DTOG_RX);
        }

        /**
         * @brief Set endpoint`s RX_DTOG flag
         *
         * @par Returns
         *  Nothing
        */
        static void SetRxDtog()
        {
            Toggle<USB_EP_DTOG_RX, USB_EP_CTR_RX | USB_EP_CTR_TX>();
        }

        /**
         * @brief Endpoint interrupt handler
         *
         * @par Returns
         *  Nothing
        */
        static void Handler();
    private:

        template<EndpointRegisterType Mask, EndpointRegisterType ExtraBits>
        static void Toggle()
        {
            EndpointRegisterType toggleMask = Reg::Get() & USB_EPREG_MASK;
            Reg::Set(toggleMask | Mask | ExtraBits);
        }

        template<EndpointRegisterType Mask, EndpointRegisterType ExtraBits = 0>
        static void ToogleAndSet(EndpointRegisterType Bit)
        {
            EndpointRegisterType toggleMask = Reg::Get() & Mask;
            toggleMask ^= Bit;
            Reg::Set(toggleMask | ExtraBits);
        }

        template<EndpointRegisterType Mask, EndpointRegisterType ExtraBits = 0>
        static void ClearRegBitMaskAndSet(EndpointRegisterType Bit)
        {
            EndpointRegisterType clrMask = Reg::Get() & Mask & ~Bit;
            Reg::Set(clrMask | ExtraBits);
        }
    };

    /**
     * @brief Implements endpoint data writer
     * 
     * @tparam _Endpoint Endpoint
     * @tparam _BufferAddress TX buffer address
     * @tparam _CountRegAddress TX_Count register address
     */
    template<typename _Endpoint, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class EndpointWriter
    {
        using BufferCountReg = PmaHalfWord<_CountRegAddress>;
    public:
        /**
         * @brief Send data
         * 
         * @details This method only writes size and set valid tx status
         * This method can be used if you writes data to endpoint TX register directly
         * 
         * @param [in] size Data size
         * 
         * @par Returns
         *  Nothing
         */
        static void SendData(uint16_t size)
        {
            BufferCountReg::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }

        /**
         * @brief Send data
         * 
         * @param [in] data Data
         * @param [in] size Data size
         * 
         * @par Returns
         *  Nothing
         */
        static void SendData(const void* data, uint16_t size)
        {
            CopyToUsbPma(reinterpret_cast<void*>(_BufferAddress), data, size);

            BufferCountReg::Set(size);
            _Endpoint::SetTxStatus(EndpointStatus::Valid);
        }
    };

    /**
     * @brief TX support (packets splitting) over the packet memory writer
     */
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    using PmaTxSupport = EndpointWithTxSupport<_Base, Endpoint<_Base, _Reg>, EndpointWriter<Endpoint<_Base, _Reg>, _BufferAddress, _CountRegAddress>>;

    /**
     * @brief Implements out (RX) endpoint
     * 
     * @tparam _Base Endpoint base
     * @tparam _Reg EPnR register
     * @tparam _BufferAddress RX buffer address
     * @tparam _CountRegAddress RX_Count register address
     */
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class OutEndpoint : public Endpoint<_Base, _Reg>, public EndpointWithRxSupport
    {
        using Base = Endpoint<_Base, _Reg>;
    public:
        static constexpr uint32_t Buffer = _BufferAddress;
        using BufferCount = PmaHalfWord<_CountRegAddress>;
        /**
         * @brief CTR Handler
         */
        static void Handler()
        {
            Base::ClearCtrRx();
            HandleRx();       
        }

        static void HandleRx();
    };

    /**
     * @brief Implements in (TX) endpoint
     * 
     * @tparam _Base Endpoint base
     * @tparam _Reg EPnR register
     * @tparam _BufferAddress TX buffer address
     * @tparam _CountRegAddress TX_Count register address
     */
    template<typename _Base, typename _Reg, uint32_t _BufferAddress, uint32_t _CountRegAddress>
    class InEndpoint : public Endpoint<_Base, _Reg>, public PmaTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>
    {
        using Base = Endpoint<_Base, _Reg>;
        using TxModule = PmaTxSupport<_Base, _Reg, _BufferAddress, _CountRegAddress>;
    public:
        static constexpr uint32_t Buffer = _BufferAddress;
        using BufferCount = PmaHalfWord<_CountRegAddress>;

        /**
         * @brief CTR Handler
         */
        static void Handler()
        {
            Base::ClearCtrTx();
            TxModule::HandleTx();
        }
    };

    /**
     * @brief Implements bidirectional endpoint
     * 
     * @tparam _Base Enpoint base
     * @tparam _Reg EPnR register
     * @tparam _TxBufferAddress TX buffer address
     * @tparam _TxCountRegAddress TX_Count register address
     * @tparam _RxBufferAddress RX buffer address
     * @tparam _RxCountRegAddress RX_Count register address
     */
    template<typename _Base, typename _Reg, uint32_t _TxBufferAddress, uint32_t _TxCountRegAddress, uint32_t _RxBufferAddress, uint32_t _RxCountRegAddress>
    class BidirectionalEndpoint : public Endpoint<_Base, _Reg>, public EndpointWithRxSupport, public PmaTxSupport<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress>
    {
        using This = BidirectionalEndpoint<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress, _RxBufferAddress, _RxCountRegAddress>;
        using Base = Endpoint<_Base, _Reg>;
        using TxModule = PmaTxSupport<_Base, _Reg, _TxBufferAddress, _TxCountRegAddress>;
    public:
        using Reg = _Reg;
        static constexpr uint32_t TxBuffer = _TxBufferAddress;
        using TxBufferCount = PmaHalfWord<_TxCountRegAddress>;
        using RxBufferCount = PmaHalfWord<_RxCountRegAddress>;
#if defined (ZHELE_USB_PMA_RX_MIRROR)
        /// Packet memory cannot be read as plain RAM (DRD: word accesses only, F1/F3: 16-bit cells
        /// spread over 32-bit words), so the received packet is mirrored into RAM and RxBuffer
        /// points to that mirror instead of the packet memory itself.
        alignas(4) static inline uint8_t _rxMirror[_Base::MaxPacketSize];
        static constexpr uint8_t* RxBuffer = _rxMirror;

        /**
         * @brief Copies the received packet from the packet memory to RxBuffer
         *
         * @par Returns
         *  Nothing
         */
        static void FetchRxBuffer()
        {
            CopyFromUsbPma(_rxMirror, reinterpret_cast<const void*>(_RxBufferAddress), RxBufferCount::Get() & 0x3ff);
        }
#else
        static constexpr uint32_t RxBuffer = _RxBufferAddress;

        /**
         * @brief Copies the received packet from the packet memory to RxBuffer (no-op, RxBuffer is the packet memory)
         *
         * @par Returns
         *  Nothing
         */
        static void FetchRxBuffer() {}
#endif

        /**
         * @brief CTR handler
         */
        static void Handler()
        {
            if(Reg::Get() & USB_EP_CTR_RX)
            {
                Base::ClearCtrRx();
                FetchRxBuffer();
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

    /**
     * @brief Implements out (RX) double-buffered bulk endpoint
     * 
     * @tparam _Base Enpoint base
     * @tparam _Reg EPnR register
     * @tparam _Buffer0Address Buffer0 address
     * @tparam _Count0RegAddress Count0 register address
     * @tparam _Buffer1Address Buffer1 address
     * @tparam _Count1RegAddress Count1 register address
     */
    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    class OutBulkDoubleBufferedEndpoint : public Endpoint<_Base, _Reg>
    {
        using Base = Endpoint<_Base, _Reg>;
    // For direct access to Buffers/CountRegs mark next fields as public
        using Reg = _Reg;
        static constexpr uint32_t Buffer0 = _Buffer0Address;
        using Buffer0Count = PmaHalfWord<_Count0RegAddress>;
        static constexpr uint32_t Buffer1 = _Buffer1Address;
        using Buffer1Count = PmaHalfWord<_Count1RegAddress>;

    public:
        /**
         * @brief Reset endpoint
         *
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            Base::Reset();
        }
    
        /**
         * @brief CTR handler
         */
        static void Handler()
        {
            Base::ClearCtrRx();

            GetCurrentBuffer() == 0
                ? HandleRx(reinterpret_cast<void*>(Buffer0), Buffer0Count::Get() & 0x3ff)
                : HandleRx(reinterpret_cast<void*>(Buffer1), Buffer1Count::Get() & 0x3ff);
            
            SwitchBuffer();
        }
    
    private:
        /**
         * @brief Switch buffer (write SW_BUF bit)
         */
        static void SwitchBuffer()
        {
            Base::SetTxDtog();
        }

        /**
         * @brief Returns current buffer for user
         * 
         * @retval 0 Buffer_0 should be used
         * @retval 1 Buffer_1 should be used
         */
        static uint8_t GetCurrentBuffer()
        {
            return (Reg::Get() & USB_EP_DTOG_TX) > 0
                ? 1
                : 0;
        }

        static void HandleRx(void* data, uint16_t size);
    };

    /**
     * @brief Implements in (TX) double-buffered bulk endpoint
     * 
     * @tparam _Base Enpoint base
     * @tparam _Reg EPnR register
     * @tparam _Buffer0Address Buffer0 address
     * @tparam _Count0RegAddress Count0 register address
     * @tparam _Buffer1Address Buffer1 address
     * @tparam _Count1RegAddress Count1 register address
     */
    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    class InBulkDoubleBufferedEndpoint : public Endpoint<_Base, _Reg>
    {
        using Base = Endpoint<_Base, _Reg>;

    // For direct access to Buffers/CountRegs mark next fields as public
        using Reg = _Reg;
        static constexpr uint32_t Buffer0 = _Buffer0Address;
        using Buffer0Count = PmaHalfWord<_Count0RegAddress>;
        static constexpr uint32_t Buffer1 = _Buffer1Address;
        using Buffer1Count = PmaHalfWord<_Count1RegAddress>;

        static const bool SendZlp = !(requires {_Base::DisableZlp;}); 
    public:
        /**
         * @brief Reset endpoint
         *
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            Base::Reset();
            Base::SetTxDtog();
        }

        static void Handler()
        {
            Base::ClearCtrTx();
            
            _bytesRemain -= _Base::MaxPacketSize;
            _dataToTransmit += _Base::MaxPacketSize;

            if(_bytesRemain > (SendZlp ? -1 : 0))
            {
                WriteData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
                return;
            }

            if(_txCompleteCallback)
            {
                Base::SetTxStatus(EndpointStatus::Nak);
                _txCompleteCallback();
            }
        }

        /**
         * @brief Send ZLP to host
         * 
         * @param [in, opt] callback Complete callback
         */
        static void SendZLP(InTransferCallback callback = nullptr)
        {
            _bytesRemain = 0;
            _txCompleteCallback = callback;
            WriteData(nullptr, 0);
            Base::SetTxStatus(EndpointStatus::Valid);
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
            _dataToTransmit = reinterpret_cast<const uint8_t*>(data);
            _bytesRemain = size;
            _txCompleteCallback = callback;

            WriteData(_dataToTransmit, _bytesRemain > _Base::MaxPacketSize ? _Base::MaxPacketSize : _bytesRemain);
            Base::SetTxStatus(EndpointStatus::Valid);
        }
    private:
        /**
         * @brief Send data
         * 
         * @param [in] data Data
         * @param [in] size Data size
         * 
         * @par Returns
         *  Nothing
         */
        static void WriteData(const void* data, uint16_t size)
        {
            CopyToUsbPma(reinterpret_cast<void*>(GetCurrentBuffer() == 0 ? Buffer0 : Buffer1), data, size);

            
            GetCurrentBuffer() == 0 ? Buffer0Count::Set(size) : Buffer1Count::Set(size);

            SwitchBuffer();
        }

        /**
         * @brief Switch buffer (write SW_BUF bit)
         */
        static void SwitchBuffer()
        {
            Base::SetRxDtog();
        }

        /**
         * @brief Returns current buffer for user
         * 
         * @retval 0 Buffer_0 should be used
         * @retval 1 Buffer_1 should be used
         */
        static uint8_t GetCurrentBuffer()
        {
            return (Reg::Get() & USB_EP_DTOG_RX) > 0
                ? 1
                : 0;
        }

        static const uint8_t* _dataToTransmit;
        static int32_t _bytesRemain;
        static InTransferCallback _txCompleteCallback;
    };

    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    const uint8_t* InBulkDoubleBufferedEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address, _Count1RegAddress>::_dataToTransmit = nullptr;
    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    int32_t InBulkDoubleBufferedEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address, _Count1RegAddress>::_bytesRemain = -1;
    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    InTransferCallback InBulkDoubleBufferedEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address, _Count1RegAddress>::_txCompleteCallback = nullptr;

    /**
     * @brief Implements bidirectional endpoint
     * 
     * @tparam _Base Enpoint base
     * @tparam _Reg EPnR register
     * @tparam _Buffer0Address Buffer0 address
     * @tparam _Count0RegAddress Count0 register address
     * @tparam _Buffer1Address Buffer1 address
     * @tparam _Count1RegAddress Count1 register address
     */
    template<typename _Base, typename _Reg, uint32_t _Buffer0Address, uint32_t _Count0RegAddress, uint32_t _Buffer1Address, uint32_t _Count1RegAddress>
    using BulkDoubleBufferedEndpoint = std::conditional_t<
        _Base::Direction == EndpointDirection::Out,
        OutBulkDoubleBufferedEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address, _Count1RegAddress>,
        InBulkDoubleBufferedEndpoint<_Base, _Reg, _Buffer0Address, _Count0RegAddress, _Buffer1Address, _Count1RegAddress>
        >;
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_PMA_ENDPOINT_H
