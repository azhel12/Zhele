/**
 * @file
 * K1921VG5T: instantiate the DMA controller, its channels and the request table.
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
#ifndef ZHELE_PLATFORM_NIIET_K1921VG5T_DMA_H
#define ZHELE_PLATFORM_NIIET_K1921VG5T_DMA_H

#include <K1921VG5T.h>

#include "../common/dma_axi.h"

#include <cstdint>

namespace Zhele
{
    /// DMA hardware request lines (РП таблица 12.1). Every block has both an RX
    /// and a TX line except the ADC sequencers (RX only).
    namespace DmaRequest
    {
        inline constexpr uint32_t Memory  = 0;
        inline constexpr uint32_t Spi0    = 1;
        inline constexpr uint32_t Spi1    = 2;
        inline constexpr uint32_t Uart0   = 3;
        inline constexpr uint32_t Uart1   = 4;
        inline constexpr uint32_t Tmr0    = 5;
        inline constexpr uint32_t Tmr1    = 6;
        inline constexpr uint32_t Tmr2    = 7;
        inline constexpr uint32_t Tmr3    = 8;
        inline constexpr uint32_t AdcSeq0 = 9;
        inline constexpr uint32_t AdcSeq1 = 10;
        inline constexpr uint32_t GpioA   = 11;
        inline constexpr uint32_t GpioB   = 12;
        inline constexpr uint32_t Pwm0    = 13;
        inline constexpr uint32_t Pwm1    = 14;
        inline constexpr uint32_t Pwm2    = 15;
        inline constexpr uint32_t Qep     = 16;
        inline constexpr uint32_t Cap0    = 17;
        inline constexpr uint32_t Cap1    = 18;
        inline constexpr uint32_t Cap2    = 19;
    } // namespace DmaRequest

    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA, DmaRegs, DMA_TypeDef);
    } // namespace Private

    using Dma1 = Private::DmaModule<Private::DmaRegs, 16, Clock::DmaClock>;

    /// DMA channel @p _Channel (0..15) serving peripheral request @p _Request.
    template<unsigned _Channel, uint32_t _Request = DmaRequest::Memory>
    using Dma1Channel = Private::DmaChannel<Dma1, _Channel, _Request,
                                            static_cast<Plic_IsrVect_TypeDef>(IsrVect_IRQ_DMA_CH0 + _Channel)>;
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG5T_DMA_H
