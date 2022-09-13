/**
 * @file
 * DMA methods implementation
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_DMA_IMPL_COMMON_H
#define ZHELE_DMA_IMPL_COMMON_H

namespace Zhele
{
    void DmaChannelData::NotifyTransferComplete()
    {
        if(transferCallback)
        {
            transferCallback(data, size, true);
        }
    }

    void DmaChannelData::NotifyError()
    {            
        if(transferCallback)
        {
            transferCallback(data, size, false);
        }
    }

    #define DMACHANNEL_TEMPLATE_ARGS template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQNumber>
    #define DMACHANNEL_TEMPLATE_QUALIFIER DmaChannel<_Module, _ChannelRegs, _Channel, _IRQNumber>

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize
    ONLY_IF_STREAM_SUPPORTED(COMMA uint8_t channel))
    {
        _Module::Enable();
        if(!TransferError())
        {
            while(!Ready())
                ;
        }
    #if defined (DMA_CCR_EN)
        _ChannelRegs()->CCR = 0;
        _ChannelRegs()->CNDTR = bufferSize;
        _ChannelRegs()->CPAR = reinterpret_cast<uint32_t>(periph);
        _ChannelRegs()->CMAR = reinterpret_cast<uint32_t>(buffer);
    #endif
    #if defined (DMA_SxCR_EN)
        _ChannelRegs()->CR = 0;
        _ChannelRegs()->NDTR = bufferSize;
        _ChannelRegs()->PAR = reinterpret_cast<uint32_t>(periph);
        _ChannelRegs()->M0AR = reinterpret_cast<uint32_t>(buffer);
    #endif
    Data.data = const_cast<void*>(buffer);
    Data.size = bufferSize;

    if(Data.transferCallback)
        mode = mode | DmaBase::TransferCompleteInterrupt | DmaBase::TransferErrorInterrupt;

    NVIC_EnableIRQ(_IRQNumber);

    #if defined (DMA_CCR_EN)
        ONLY_IF_STREAM_SUPPORTED (_Module::template SetChannelSelect<_Channel> (channel));
        _ChannelRegs()->CCR = mode | DMA_CCR_EN;
    #endif
    #if defined (DMA_SxCR_EN)
        _ChannelRegs()->CR = mode | ((channel & 0x07) << 25) | DMA_SxCR_EN;
    #endif
    }
    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::SetTransferCallback(TransferCallback callback)
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
        return _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) & ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Enable()
    {
        _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) |= ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::Disable()
    {
        _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) &= ~ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
    }

    DMACHANNEL_TEMPLATE_ARGS
    uint32_t DMACHANNEL_TEMPLATE_QUALIFIER::RemainingTransfers()
    {
        return _ChannelRegs()->ONLY_FOR_CCR(CNDTR)ONLY_FOR_SXCR(NDTR);
    }

    DMACHANNEL_TEMPLATE_ARGS
    void* DMACHANNEL_TEMPLATE_QUALIFIER::PeriphAddress()
    {
        return reinterpret_cast<void *>(_ChannelRegs()->ONLY_FOR_CCR(CPAR)ONLY_FOR_SXCR(PAR));
    }

    DMACHANNEL_TEMPLATE_ARGS
    void* DMACHANNEL_TEMPLATE_QUALIFIER::MemAddress()
    {
        return reinterpret_cast<void *>(_ChannelRegs()->ONLY_FOR_CCR(CMAR)ONLY_FOR_SXCR(M0AR));
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::TransferError()
    {
        return _Module::template TransferError<_Channel>();
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::HalfTransfer()
    {
        return _Module::template HalfTransfer<_Channel>();
    }

    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::TransferComplete()
    {
        return _Module::template TransferComplete<_Channel>();
    }
#if defined(DMA_CCR_EN)
    DMACHANNEL_TEMPLATE_ARGS
    bool DMACHANNEL_TEMPLATE_QUALIFIER::Interrupt()
    {
        return _Module::template Interrupt<_Channel>();
    }
#endif
    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearFlags()
    {
        _Module::template ClearChannelFlags<_Channel>();
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearTransferError()
    {
        _Module::template ClearTransferError<_Channel>();
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearHalfTransfer()
    {
        _Module::template ClearHalfTransfer<_Channel>();
    }

    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearTransferComplete() 
    {
        _Module::template ClearTransferComplete<_Channel>();
    }
#if defined(DMA_CCR_EN)
    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::ClearInterrupt()
    {
        _Module::template ClearInterrupt<_Channel>();
    }
#endif
    DMACHANNEL_TEMPLATE_ARGS
    void DMACHANNEL_TEMPLATE_QUALIFIER::IrqHandler()
    {
        if(TransferComplete())
        {
            ClearFlags();
            
            if(static_cast<uint32_t>(_ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) & Mode::Circular) == 0)
                Disable();

            Data.NotifyTransferComplete();
        }
        if(TransferError())
        {
            ClearFlags();

            if(static_cast<uint32_t>(_ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) & Mode::Circular) == 0)
                Disable();

            Data.NotifyError();
        }
    }

    #define DMAMODULE_TEMPLATE_ARGS template<typename _DmaRegs, typename _Clock, unsigned _Channels>
    #define DMAMODULE_TEMPLATE_QUALIFIER DmaModule<_DmaRegs, _Clock, _Channels>

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum, typename DMAMODULE_TEMPLATE_QUALIFIER::Flags FlagMask>
    bool DMAMODULE_TEMPLATE_QUALIFIER::ChannelFlag()
    {
    #if defined(DMA_CCR_EN)
        return _DmaRegs()->ISR & (static_cast<uint32_t>(FlagMask) << ((ChannelNum - 1) * 4));
    #endif
    #if defined(DMA_SxCR_EN)
        if constexpr(ChannelNum <= 1)
        {
            return _DmaRegs()->LISR & (static_cast<uint32_t>(FlagMask) << (ChannelNum * 6));
        }
        if constexpr(2 <= ChannelNum && ChannelNum <= 3)
        {
            return _DmaRegs()->LISR & (static_cast<uint32_t>(FlagMask) << (4 + ChannelNum * 6));
        }
        if constexpr(4 <= ChannelNum && ChannelNum <= 5)
        {
            return _DmaRegs()->HISR & (static_cast<uint32_t>(FlagMask) << ((ChannelNum - 4) * 6));
        }
        if constexpr(6 <= ChannelNum && ChannelNum <= 7)
        {
            return _DmaRegs()->HISR & (static_cast<uint32_t>(FlagMask) << (4 + (ChannelNum - 4) * 6));
        }
        return false;
    #endif
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum, typename DMAMODULE_TEMPLATE_QUALIFIER::Flags FlagMask>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearChannelFlag()
    {
    #if defined(DMA_CCR_EN)
        _DmaRegs()->IFCR |= (static_cast<uint32_t>(FlagMask) << ((ChannelNum - 1) * 4));
    #endif
    #if defined(DMA_SxCR_EN)
        if constexpr(ChannelNum <= 1)
        {
            _DmaRegs()->LIFCR |= (static_cast<uint32_t>(FlagMask) << (ChannelNum * 6));
        }
        if constexpr(2 <= ChannelNum && ChannelNum <= 3)
        {
            _DmaRegs()->LIFCR |= (static_cast<uint32_t>(FlagMask) << (4 + ChannelNum * 6));
        }
        if constexpr(4 <= ChannelNum && ChannelNum <= 5)
        {
            _DmaRegs()->HIFCR |= (static_cast<uint32_t>(FlagMask) << ((ChannelNum - 4) * 6));
        }
        if constexpr(6 <= ChannelNum && ChannelNum <= 7)
        {
            _DmaRegs()->HIFCR |= (static_cast<uint32_t>(FlagMask) << (4 + (ChannelNum - 4) * 6));
        }
    #endif
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::TransferError()
    {
        return ChannelFlag<ChannelNum, Flags::TransferError>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::HalfTransfer()
    {
        return ChannelFlag<ChannelNum, Flags::HalfTransfer>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::TransferComplete()
    {
        return ChannelFlag<ChannelNum, Flags::TransferComplete>();
    }

#if defined(DMA_SxCR_EN)
    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::FifoError()
    {
        return ChannelFlag<ChannelNum, Flags::FifoError>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::DirectError()
    {
        return ChannelFlag<ChannelNum, Flags::DirectError>();
    }
#endif
#if defined(DMA_CCR_EN)
    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    bool DMAMODULE_TEMPLATE_QUALIFIER::Interrupt()
    {
        return ChannelFlag<ChannelNum, Flags::Global>();
    }
#endif
    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearChannelFlags()
    {
        ClearChannelFlag<ChannelNum, Flags::All>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearTransferError()
    {
        ClearChannelFlag<ChannelNum, Flags::TransferError>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearHalfTransfer()
    {
        ClearChannelFlag<ChannelNum, Flags::HalfTransfer>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearTransferComplete()
    {
        ClearChannelFlag<ChannelNum, Flags::TransferComplete>();
    }
#if defined(DMA_CCR_EN)
    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearInterrupt()
    {
        ClearChannelFlag<ChannelNum, Flags::Global>();
    }
#endif
#if defined(DMA_SxCR_EN)
    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearFifoError()
    {
        ClearChannelFlag<ChannelNum, Flags::FifoError>();
    }

    DMAMODULE_TEMPLATE_ARGS
    template<int ChannelNum>
    void DMAMODULE_TEMPLATE_QUALIFIER::ClearDirectError()
    {
        ClearChannelFlag<ChannelNum, Flags::DirectError>();
    }
#endif

    DMAMODULE_TEMPLATE_ARGS
    void DMAMODULE_TEMPLATE_QUALIFIER::Enable()
    {
        _Clock::Enable();
        // See note.
        DmaDummy();
    }

    DMAMODULE_TEMPLATE_ARGS
    void DMAMODULE_TEMPLATE_QUALIFIER::Disable()
    {
        _Clock::Disable();
    }

    #if defined (DMA_CSELR_C1S)
        DMAMODULE_TEMPLATE_ARGS
        template<uint8_t channel>
        void DMAMODULE_TEMPLATE_QUALIFIER::SetChannelSelect(uint8_t channelSelect)
        {
            static const uint32_t ChannelSelectRegisterOffset = 0x0a8;

            *(reinterpret_cast<uint32_t*>(_DmaRegs::Get()) + ChannelSelectRegisterOffset) = 
                (*(reinterpret_cast<uint32_t*>(_DmaRegs::Get()) + ChannelSelectRegisterOffset) & ~(0xf << 4 * channel)) 
                | (channelSelect << 4 * channel);
        }
    #endif
}
#endif //! ZHELE_DMA_IMPL_COMMON_H