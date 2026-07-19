/**
 * @file
 * SPI master for CH32 — methods implementation.
 *
 * @author Aleksei Zhelonkin (based on the STM32 spi implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_SPI_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_SPI_H

namespace Zhele
{
    namespace Private
    {
        #define SPI_TEMPLATE_ARGS template< \
            typename _Regs, \
            typename _Clock, \
            typename _DmaTx, \
            typename _DmaRx, \
            uint32_t _RemapMask>

        #define SPI_TEMPLATE_QUALIFIER Spi<_Regs, _Clock, _DmaTx, _DmaRx, _RemapMask>

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::Enable()
        {
            _Regs()->CTLR1 |= SPI_CTLR1_SPE;
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::Disable()
        {
            _Regs()->CTLR1 &= ~SPI_CTLR1_SPE;
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::Init(SPI_TEMPLATE_QUALIFIER::ClockDivider divider, SPI_TEMPLATE_QUALIFIER::Mode mode)
        {
            _Clock::Enable();
            _Regs()->CTLR1 = static_cast<uint16_t>(divider) | static_cast<uint16_t>(mode);
            _Regs()->CTLR2 = 0;
            Enable();
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetDivider(SPI_TEMPLATE_QUALIFIER::ClockDivider divider)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_BR)) | static_cast<uint16_t>(divider);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetClockPolarity(SPI_TEMPLATE_QUALIFIER::ClockPolarity clockPolarity)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_CPOL)) | static_cast<uint16_t>(clockPolarity);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetClockPhase(SPI_TEMPLATE_QUALIFIER::ClockPhase clockPhase)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_CPHA)) | static_cast<uint16_t>(clockPhase);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetBitOrder(SPI_TEMPLATE_QUALIFIER::BitOrder bitOrder)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_LSBFIRST)) | static_cast<uint16_t>(bitOrder);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetDataSize(SPI_TEMPLATE_QUALIFIER::DataSize dataSize)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_DFF)) | static_cast<uint16_t>(dataSize);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetSlaveControl(SPI_TEMPLATE_QUALIFIER::SlaveControl slaveControl)
        {
            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~static_cast<uint16_t>(SPI_CTLR1_SSM)) | static_cast<uint16_t>(slaveControl);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SetSS()
        {
            _Regs()->CTLR1 |= SPI_CTLR1_SSI;
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::ClearSS()
        {
            _Regs()->CTLR1 &= ~SPI_CTLR1_SSI;
        }

        SPI_TEMPLATE_ARGS
        bool SPI_TEMPLATE_QUALIFIER::Busy()
        {
            return (_Regs()->STATR & SPI_STATR_BSY) != 0;
        }

        SPI_TEMPLATE_ARGS
        uint16_t SPI_TEMPLATE_QUALIFIER::Send(uint16_t value)
        {
            while ((_Regs()->STATR & SPI_STATR_TXE) == 0)
                ;
            _Regs()->DATAR = value;
            while ((_Regs()->STATR & SPI_STATR_RXNE) == 0)
                ;
            return _Regs()->DATAR;
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::Write(uint16_t data)
        {
            Send(data);
        }

        SPI_TEMPLATE_ARGS
        uint16_t SPI_TEMPLATE_QUALIFIER::Read()
        {
            return Send(0xFFFF);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::SendAsync(const void* transmitBuffer, void* receiveBuffer, size_t bufferSize, SPI_TEMPLATE_QUALIFIER::TransferCallback callback)
        {
            _DmaRx::ClearTransferComplete();
            _Regs()->CTLR2 |= (SPI_CTLR2_RXDMAEN | SPI_CTLR2_TXDMAEN);
            auto dataSize = DmaDataSize();
            _DmaRx::SetTransferCallback(callback);
            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | dataSize, receiveBuffer, &_Regs()->DATAR, bufferSize);
            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | dataSize, transmitBuffer, &_Regs()->DATAR, bufferSize);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::WriteAsync(const void* data, uint16_t size, SPI_TEMPLATE_QUALIFIER::TransferCallback callback)
        {
            _DmaTx::ClearTransferComplete();
            _Regs()->CTLR2 |= SPI_CTLR2_TXDMAEN;
            _DmaTx::SetTransferCallback(callback);
            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | DmaDataSize(), data, &_Regs()->DATAR, size);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::WriteAsyncNoIncrement(const void* data, uint16_t size, SPI_TEMPLATE_QUALIFIER::TransferCallback callback)
        {
            _DmaTx::ClearTransferComplete();
            _Regs()->CTLR2 |= SPI_CTLR2_TXDMAEN;
            _DmaTx::SetTransferCallback(callback);
            _DmaTx::Transfer(_DmaTx::Mem2Periph | DmaDataSize(), data, &_Regs()->DATAR, size);
        }

        SPI_TEMPLATE_ARGS
        void SPI_TEMPLATE_QUALIFIER::ReadAsync(void* receiveBuffer, size_t bufferSize, SPI_TEMPLATE_QUALIFIER::TransferCallback callback)
        {
            static const uint16_t dummy = 0xFFFF;
            _DmaRx::ClearTransferComplete();
            _Regs()->CTLR2 |= (SPI_CTLR2_RXDMAEN | SPI_CTLR2_TXDMAEN);
            auto dataSize = DmaDataSize();
            _DmaRx::SetTransferCallback(callback);
            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | dataSize, receiveBuffer, &_Regs()->DATAR, bufferSize);
            _DmaTx::Transfer(_DmaTx::Mem2Periph | dataSize, &dummy, &_Regs()->DATAR, bufferSize);
        }

        SPI_TEMPLATE_ARGS
        template<typename MosiPin, typename MisoPin, typename SckPin, typename SsPin, uint8_t Remap>
        void SPI_TEMPLATE_QUALIFIER::SelectPins()
        {
            SckPin::Port::Enable();
            SckPin::template SetConfiguration<SckPin::Port::Configuration::AltFunc>();
            SckPin::template SetDriverType<SckPin::Port::DriverType::PushPull>();
            SckPin::template SetSpeed<SckPin::Port::Speed::Fast>();

            if constexpr (!std::is_same_v<MosiPin, IO::NullPin>)
            {
                MosiPin::Port::Enable();
                MosiPin::template SetConfiguration<MosiPin::Port::Configuration::AltFunc>();
                MosiPin::template SetDriverType<MosiPin::Port::DriverType::PushPull>();
                MosiPin::template SetSpeed<MosiPin::Port::Speed::Fast>();
            }

            if constexpr (!std::is_same_v<MisoPin, IO::NullPin>)
            {
                MisoPin::Port::Enable();
                MisoPin::template SetConfiguration<MisoPin::Port::Configuration::In>();
                MisoPin::template SetPullMode<MisoPin::Port::PullMode::PullUp>();
            }

            if constexpr (!std::is_same_v<SsPin, IO::NullPin>)
            {
                SsPin::Port::Enable();
                SsPin::template SetConfiguration<SsPin::Port::Configuration::AltFunc>();
                SsPin::template SetDriverType<SsPin::Port::DriverType::PushPull>();
            }

            if constexpr (_RemapMask != 0)
            {
                Clock::Apb2PeriphClockEnable::Or(RCC_AFIOEN);
                if constexpr (Remap != 0)
                    AFIO->PCFR1 |= _RemapMask;
                else
                    AFIO->PCFR1 &= ~_RemapMask;
            }
        }

        SPI_TEMPLATE_ARGS
        typename _DmaTx::Mode SPI_TEMPLATE_QUALIFIER::DmaDataSize()
        {
            return (_Regs()->CTLR1 & SPI_CTLR1_DFF)
                ? static_cast<typename _DmaTx::Mode>(_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
                : static_cast<typename _DmaTx::Mode>(_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);
        }

        #undef SPI_TEMPLATE_ARGS
        #undef SPI_TEMPLATE_QUALIFIER
    }
}

#endif // ZHELE_PLATFORM_CH32_COMMON_IMPL_SPI_H
