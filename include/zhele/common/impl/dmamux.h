/**
 * @file
 * DMAMUX methods implementation
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_DMAMUX_IMPL_COMMON_H
#define ZHELE_DMAMUX_IMPL_COMMON_H

#include <cstdint>

namespace Zhele
{
    #define DMAMUXIMPL_TEMPLATE_ARGS template<typename _BaseRegs, std::convertible_to<uint32_t> _SyncInput, std::convertible_to<uint32_t> _RequestInput>
    #define DMAMUXIMPL_TEMPLATE_QUALIFIER DmaMux<_BaseRegs, _SyncInput, _RequestInput>

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline void DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::SetSyncInput(_SyncInput input)
    {
        SyncInputBitfield::Set(static_cast<uint32_t>(input));
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline _SyncInput DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::GetSyncInput()
    {
        return static_cast<_SyncInput>(SyncInputBitfield::Get());
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline void DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::SetSyncPolarity(SyncPolarity polarity)
    {
        SyncPolarityBitfield::Set(static_cast<uint32_t>(polarity));
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::SyncPolarity DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::GetSyncPolarity()
    {
        return static_cast<SyncPolarity>(SyncPolarityBitfield::Get());
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline void DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::EnableSynchronization()
    {
        ChannelRegs::Get()[_ChannelNumber].CCR |= DMAMUX_CxCR_SE;
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline void DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::DisableSynchronization()
    {
        ChannelRegs::Get()[_ChannelNumber].CCR &= ~DMAMUX_CxCR_SE;
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline void DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::SelectRequestInput(_RequestInput input)
    {
        RequestPolarityBitfield::Set(static_cast<uint32_t>(input));
    }

    DMAMUXIMPL_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    inline _RequestInput DMAMUXIMPL_TEMPLATE_QUALIFIER::Channel<_ChannelNumber>::GetRequestInput()
    {
        return static_cast<_RequestInput>(RequestPolarityBitfield::Get());
    }
}
#endif  //! ZHELE_DMAMUX_IMPL_COMMON_H