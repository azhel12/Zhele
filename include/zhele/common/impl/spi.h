/**
 * @file
 * SPI methods implementation
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_SPI_IMPL_COMMON_H
#define ZHELE_SPI_IMPL_COMMON_H

namespace Zhele::Private
{
    #define SPI_TEMPLATE_ARGS template< \
        typename _Regs, \
        typename _Clock, \
        typename _MosiPins, \
        typename _MisoPins, \
        typename _ClockPins, \
        typename _SsPins, \
        typename _DmaTx, \
        typename _DmaRx>

    #define SPI_TEMPLATE_QUALIFIER Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::Enable()
    {
        _Regs()->CR1 |= SPI_CR1_SPE;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::Disable()
    {
        _Regs()->CR1 &= ~SPI_CR1_SPE;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::Init(SPI_TEMPLATE_QUALIFIER::ClockDivider divider, SPI_TEMPLATE_QUALIFIER::Mode mode)
    {
        _Clock::Enable();
        _Regs()->CR1 = static_cast<unsigned>(divider) | mode;
        _Regs()->CR2 = (mode >> 16) | SPI_CR2_SSOE;
        SetDataSize(DataSize::DataSize8);
#if defined (SPI_I2SCFGR_I2SMOD)
        _Regs()->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
#endif
        Enable();
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetDivider(SPI_TEMPLATE_QUALIFIER::ClockDivider divider)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_BR) | divider;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetClockPolarity(SPI_TEMPLATE_QUALIFIER::ClockPolarity clockPolarity)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_CPOL) | clockPolarity;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetClockPhase(SPI_TEMPLATE_QUALIFIER::ClockPhase clockPhase)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_CPHA) | clockPhase;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetBitOrder(SPI_TEMPLATE_QUALIFIER::BitOrder bitOrder)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_LSBFIRST) | bitOrder;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetDataSize(SPI_TEMPLATE_QUALIFIER::DataSize dataSize)
    {
        #if defined (SPI_CR1_DFF)
            _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_DFF_Msk) | dataSize;
        #else
            _Regs()->CR2 = (_Regs()->CR2 & ~SPI_CR2_DS) | dataSize;
            #if defined(SPI_CR2_FRXTH)
                if(dataSize <= DataSize8)
                {
                    _Regs()->CR2 |= SPI_CR2_FRXTH;
                }
            #endif
        #endif
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetSlaveControl(SPI_TEMPLATE_QUALIFIER::SlaveControl slaveControl)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~SPI_CR1_SSM) | slaveControl;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SetSS()
    {
        _Regs()->CR1 |= SPI_CR1_SSI;
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::ClearSS()
    {
        _Regs()->CR1 &= ~SPI_CR1_SSI;
    }

    SPI_TEMPLATE_ARGS
    bool SPI_TEMPLATE_QUALIFIER::Busy()
    {
        return (_Regs()->SR & SPI_SR_BSY) > 0;
    }

    SPI_TEMPLATE_ARGS
    uint16_t SPI_TEMPLATE_QUALIFIER::Send(uint16_t value)
    {
        while ((_Regs()->SR & SPI_SR_TXE) == 0);

    #if defined(SPI_CR1_DFF)
        if(_Regs()->CR1 & SPI_CR1_DFF)
    #else
        if((_Regs()->CR2 & SPI_CR2_DS) > DataSize8)
    #endif
        {
            _Regs()->DR = value;
        }
        else
        {
            *(__IO uint8_t*)&_Regs()->DR = static_cast<uint8_t>(value);
        }
        
        while ((_Regs()->SR & SPI_SR_RXNE) == 0);
    #if defined(SPI_CR1_DFF)
        if(_Regs()->CR1 & SPI_CR1_DFF)
    #else
        if((_Regs()->CR2 & SPI_CR2_DS) > DataSize8)
    #endif
        {
            return _Regs()->DR;
        }
        else
        {
            return *(__IO uint8_t*)&_Regs()->DR;
        }
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::SendAsync(void* transmitBuffer, void* receiveBuffer, size_t bufferSize, TransferCallback callback)
    {
        _DmaRx::ClearTransferComplete();
        _Regs()->CR2 |= (SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);
        auto dataSize = 
        #if defined(SPI_CR1_DFF)
            (_Regs()->CR1 & SPI_CR1_DFF) > 0
        #else
            (_Regs()->CR2 & SPI_CR2_DS) > DataSize8
        #endif
            ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
            : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);
        _DmaRx::SetTransferCallback(callback);
        _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular | dataSize, receiveBuffer, &_Regs()->DR, bufferSize);

        _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaRx::MemIncrement | dataSize, transmitBuffer, &_Regs()->DR, bufferSize);
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::Write(uint16_t data)
    {
        Send(data);
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::WriteAsync(const void* data, uint16_t size, TransferCallback callback)
    {
        _DmaTx::ClearTransferComplete();
        _Regs()->CR2 |= SPI_CR2_TXDMAEN;
        typename _DmaTx::Mode dataSize = 
        #if defined(SPI_CR1_DFF)
            (_Regs()->CR1 & SPI_CR1_DFF) > 0
        #else
            (_Regs()->CR2 & SPI_CR2_DS) > DataSize8
        #endif
            ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
            : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

        _DmaTx::SetTransferCallback(callback);
        _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | dataSize, data, &_Regs()->DR, size);
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::WriteAsyncNoIncrement(const void* data, uint16_t size, TransferCallback callback)
    {
        _DmaTx::ClearTransferComplete();
        _Regs()->CR2 |= SPI_CR2_TXDMAEN;
        auto dataSize = 
        #if defined(SPI_CR1_DFF)
            (_Regs()->CR1 & SPI_CR1_DFF) > 0
        #else
            (_Regs()->CR2 & SPI_CR2_DS) > DataSize8
        #endif
            ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
            : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

        _DmaTx::SetTransferCallback(callback);
        _DmaTx::Transfer(_DmaTx::Mem2Periph | dataSize, data, &_Regs()->DR, size);
    }

    SPI_TEMPLATE_ARGS
    uint16_t SPI_TEMPLATE_QUALIFIER::Read()
    {
        return Send(0xffff);
    }

    SPI_TEMPLATE_ARGS
    void SPI_TEMPLATE_QUALIFIER::ReadAsync(void* receiveBuffer, size_t bufferSize, TransferCallback callback)
    {
        _DmaRx::ClearTransferComplete();
        _Regs()->CR2 |= (SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);
        auto dataSize = 
        #if defined(SPI_CR1_DFF)
            (_Regs()->CR1 & SPI_CR1_DFF) > 0
        #else
            (_Regs()->CR2 & SPI_CR2_DS) > DataSize8
        #endif
            ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
            : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);
        _DmaRx::SetTransferCallback(callback);
        _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular | dataSize, receiveBuffer, &_Regs()->DR, bufferSize);

        // Send dummmy value
        uint16_t dummy = 0xffff;
        _DmaTx::Transfer(_DmaTx::Mem2Periph | dataSize, &dummy, &_Regs()->DR, bufferSize);
    }
}
#endif //! ZHELE_SPI_IMPL_COMMON_H