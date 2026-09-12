/**
 * @file
 * K1921VG7T: instantiate the DMA controller, its channels and the request table.
 *
 * Channels are interchangeable — the peripheral is selected by the request number
 * a channel is bound to (РП таблица 12.1), so a peripheral is handed a channel
 * alias carrying both:
 *
 * @code
 * using UartTxDma = Zhele::Dma1Channel<0, Zhele::DmaRequest::Uart0>;
 * using Uart      = Zhele::Usart0<UartTxDma>;
 * @endcode
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG7T_DMA_H
#define ZHELE_PLATFORM_NIIET_K1921VG7T_DMA_H

#include <K1921VG7T.h>

#include "../common/dma_axi.h"

#include <cstdint>

namespace Zhele
{
    /// DMA hardware request lines (РП таблица 12.1). Every block has both an RX
    /// and a TX line except ADC sequencers (RX only) and the DAC (TX only).
    namespace DmaRequest
    {
        inline constexpr uint32_t Memory  = 0;
        inline constexpr uint32_t Spi0    = 1;
        inline constexpr uint32_t Spi1    = 2;
        inline constexpr uint32_t Spi2    = 3;
        inline constexpr uint32_t Uart0   = 4;
        inline constexpr uint32_t Uart1   = 5;
        inline constexpr uint32_t Tmr32_0 = 6;
        inline constexpr uint32_t Tmr32_1 = 7;
        inline constexpr uint32_t Tmr16_0 = 8;
        inline constexpr uint32_t Tmr16_1 = 9;
        inline constexpr uint32_t Tmr16_2 = 10;
        inline constexpr uint32_t AdcSeq0 = 11;
        inline constexpr uint32_t AdcSeq1 = 12;
        inline constexpr uint32_t AdcSeq2 = 13;
        inline constexpr uint32_t AdcSeq3 = 14;
        inline constexpr uint32_t GpioA   = 15;
        inline constexpr uint32_t GpioB   = 16;
        inline constexpr uint32_t Dac     = 17;
    } // namespace DmaRequest

    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA, DmaRegs, DMA_TypeDef);
    } // namespace Private

    using Dma1 = Private::DmaModule<Private::DmaRegs, 8, Clock::DmaClock>;

    /// DMA channel @p _Channel (0..7) serving peripheral request @p _Request.
    template<unsigned _Channel, uint32_t _Request = DmaRequest::Memory>
    using Dma1Channel = Private::DmaChannel<Dma1, _Channel, _Request,
                                            static_cast<Plic_IsrVect_TypeDef>(IsrVect_IRQ_DMA_CH0 + _Channel)>;
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG7T_DMA_H
