/**
 * @file
 * K1921VG015: instantiate the µDMA module, control table and named channels.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG015_DMA_H
#define ZHELE_PLATFORM_NIIET_K1921VG015_DMA_H

#include <K1921VG015.h>

#include "../common/dma.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA, DmaRegs, DMA_TypeDef);

        /**
         * @brief The µDMA channel-control table.
         *
         * Must be 1 KB aligned (primary structure base address bits [9:0] are
         * ignored by the controller). One shared instance for the whole system.
         */
        struct DmaControlTable
        {
            static DMA_CtrlData_TypeDef& Get()
            {
                alignas(1024) static DMA_CtrlData_TypeDef table{};
                return table;
            }
        };
    } // namespace Private

    using Dma1 = DmaModule<Private::DmaRegs, Private::DmaControlTable>;

    // Fixed peripheral channels (see DMA_CH_* in the CMSIS header).
    // The IRQ vector is shared 3-channels-each: DMA2 = ch6-8, DMA3 = ch9-11, DMA4 = ch12-14.
    using DmaUart0Tx = DmaChannel<Dma1, DMA_CH_UART0TX, IsrVect_IRQ_DMA2>; // ch 8
    using DmaUart1Tx = DmaChannel<Dma1, DMA_CH_UART1TX, IsrVect_IRQ_DMA3>; // ch 9
    using DmaUart2Tx = DmaChannel<Dma1, DMA_CH_UART2TX, IsrVect_IRQ_DMA3>; // ch 10
    using DmaUart0Rx = DmaChannel<Dma1, DMA_CH_UART0RX, IsrVect_IRQ_DMA3>; // ch 11
    using DmaUart1Rx = DmaChannel<Dma1, DMA_CH_UART1RX, IsrVect_IRQ_DMA4>; // ch 12
    using DmaUart2Rx = DmaChannel<Dma1, DMA_CH_UART2RX, IsrVect_IRQ_DMA4>; // ch 13
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG015_DMA_H
