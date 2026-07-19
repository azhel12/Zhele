/**
 * @file
 * DMA methods implementation for CH32.
 */

#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_DMA_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_DMA_H

namespace Zhele
{
    #define DMACHANNEL_TEMPLATE_ARGS template< \
        typename _Module, \
        typename _ChannelRegs, \
        unsigned _Channel, \
        IRQn_Type _IRQNumber>

    #define DMACHANNEL_TEMPLATE_QUALIFIER DmaChannel<_Module, _ChannelRegs, _Channel, _IRQNumber>

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize)
    {
        Module::Enable();

        Disable();
        ClearFlags();

        _ChannelRegs()->PADDR = reinterpret_cast<uint32_t>(periph);
        _ChannelRegs()->MADDR = reinterpret_cast<uint32_t>(buffer);
        _ChannelRegs()->CNTR  = bufferSize;

        Data.data = const_cast<void*>(buffer);
        Data.size = static_cast<uint16_t>(bufferSize);

        uint32_t cfgr = static_cast<uint32_t>(mode) | DMA_CFGR1_EN;
        if (Data.transferCallback != nullptr)
        {
            cfgr |= DMA_CFGR1_TCIE | DMA_CFGR1_TEIE;
            NVIC_EnableIRQ(_IRQNumber);
        }

        _ChannelRegs()->CFGR = cfgr;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::SetTransferCallback(DmaChannelData::TransferCallback callback)
    {
        Data.transferCallback = callback;
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::Ready()
    {
        return RemainingTransfers() == 0 || !Enabled() || TransferComplete();
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::Enabled()
    {
        return (_ChannelRegs()->CFGR & DMA_CFGR1_EN) != 0;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Enable()
    {
        _ChannelRegs()->CFGR |= DMA_CFGR1_EN;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Disable()
    {
        _ChannelRegs()->CFGR &= ~DMA_CFGR1_EN;
    }

    DMACHANNEL_TEMPLATE_ARGS
    uint32_t DMACHANNEL_TEMPLATE_QUALIFIER::RemainingTransfers()
    {
        return _ChannelRegs()->CNTR;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void* DMACHANNEL_TEMPLATE_QUALIFIER::PeriphAddress()
    {
        return reinterpret_cast<void*>(_ChannelRegs()->PADDR);
    }

    DMACHANNEL_TEMPLATE_ARGS
    void* DMACHANNEL_TEMPLATE_QUALIFIER::MemAddress()
    {
        return reinterpret_cast<void*>(_ChannelRegs()->MADDR);
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::TransferError()
    {
        return (_Module::Regs()->INTFR & (DMA_TEIF1 << FlagsShift)) != 0;
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::HalfTransfer()
    {
        return (_Module::Regs()->INTFR & (DMA_HTIF1 << FlagsShift)) != 0;
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::TransferComplete()
    {
        return (_Module::Regs()->INTFR & (DMA_TCIF1 << FlagsShift)) != 0;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearFlags()
    {
        _Module::Regs()->INTFCR = (DMA_GIF1 | DMA_TCIF1 | DMA_HTIF1 | DMA_TEIF1) << FlagsShift;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearTransferError()
    {
        _Module::Regs()->INTFCR = DMA_TEIF1 << FlagsShift;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearHalfTransfer()
    {
        _Module::Regs()->INTFCR = DMA_HTIF1 << FlagsShift;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearTransferComplete()
    {
        _Module::Regs()->INTFCR = DMA_TCIF1 << FlagsShift;
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::IrqHandler()
    {
        if (TransferComplete())
        {
            ClearFlags();
            Disable();
            Data.NotifyTransferComplete();
        }
        else if (TransferError())
        {
            ClearFlags();
            Disable();
            Data.NotifyError();
        }
    }

    DMACHANNEL_TEMPLATE_ARGS
    DmaChannelData DMACHANNEL_TEMPLATE_QUALIFIER::Data;

    #undef DMACHANNEL_TEMPLATE_ARGS
    #undef DMACHANNEL_TEMPLATE_QUALIFIER

    #define DMAMODULE_TEMPLATE_ARGS template< \
        typename _DmaRegs, \
        typename _Clock, \
        unsigned _Channels>

    #define DMAMODULE_TEMPLATE_QUALIFIER DmaModule<_DmaRegs, _Clock, _Channels>

    DMAMODULE_TEMPLATE_ARGS
    auto DMAMODULE_TEMPLATE_QUALIFIER::Regs()
    {
        return _DmaRegs();
    }

    DMAMODULE_TEMPLATE_ARGS
    void DMAMODULE_TEMPLATE_QUALIFIER::Enable()
    {
        _Clock::Enable();
    }

    DMAMODULE_TEMPLATE_ARGS
    void DMAMODULE_TEMPLATE_QUALIFIER::Disable()
    {
        _Clock::Disable();
    }

    #undef DMAMODULE_TEMPLATE_ARGS
    #undef DMAMODULE_TEMPLATE_QUALIFIER
}
#endif //! ZHELE_PLATFORM_CH32_COMMON_IMPL_DMA_H
