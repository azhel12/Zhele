#include <common/macro_utils/declarations.h>
#include <dma.h>

using namespace Zhele;

template<> DmaChannelData Zhele::Dma1Channel1::ChannelData = { };
template<> DmaChannelData Zhele::Dma1Channel2::ChannelData = { };
template<> DmaChannelData Zhele::Dma1Channel3::ChannelData = { };
template<> DmaChannelData Zhele::Dma1Channel4::ChannelData = { };
template<> DmaChannelData Zhele::Dma1Channel5::ChannelData = { };

#if defined (DMA1_Channel6)
template<> DmaChannelData Zhele::Dma1Channel6::ChannelData = { };
#endif
#if defined (DMA1_Channel7)
template<> DmaChannelData Zhele::Dma1Channel7::ChannelData = { };
#endif
