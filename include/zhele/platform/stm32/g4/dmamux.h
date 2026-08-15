/**
 * @file
 * Implement DMAMUX module for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_DMAMUX_H
#define ZHELE_PLATFORM_STM32_G4_DMAMUX_H

#include <stm32g4xx.h>
#include "../common/dmamux.h"

namespace Zhele
{
    // Enumerator names are prefixed (SyncExtiN, not ExtiN) because DmamuxSyncInput is an
    // unscoped enum in namespace Zhele (required by DmaMux's std::convertible_to<uint32_t>
    // constraint) and would otherwise collide with the Exti0..Exti15 class aliases in exti.h.
    enum DmamuxSyncInput
    {
        SyncExti0 = 0,
        SyncExti1 = 1,
        SyncExti2 = 2,
        SyncExti3 = 3,
        SyncExti4 = 4,
        SyncExti5 = 5,
        SyncExti6 = 6,
        SyncExti7 = 7,
        SyncExti8 = 8,
        SyncExti9 = 9,
        SyncExti10 = 10,
        SyncExti11 = 11,
        SyncExti12 = 12,
        SyncExti13 = 13,
        SyncExti14 = 14,
        SyncExti15 = 15,
        DmamuxEvent0 = 16,
        DmamuxEvent1 = 17,
        DmamuxEvent2 = 18,
        DmamuxEvent3 = 19,
        Lptim1Out = 20,
    };

    // Adc1..Adc5 are prefixed (AdcReqN, not AdcN) to avoid colliding with the Adc1..Adc5
    // class template aliases in adc.h (same unscoped-enum issue as DmamuxSyncInput above).
    /// DMAMUX request inputs (RM0440 Table "DMAMUX: assignment of multiplexer inputs to resources")
    enum DmamuxRequestInput
    {
        Mem2Mem = 0,
        Generator0 = 1,
        Generator1 = 2,
        Generator2 = 3,
        Generator3 = 4,
        AdcReq1 = 5,
        Dac1Ch1 = 6,
        Dac1Ch2 = 7,
        Tim6Up = 8,
        Tim7Up = 9,
        Spi1Rx = 10,
        Spi1Tx = 11,
        Spi2Rx = 12,
        Spi2Tx = 13,
        Spi3Rx = 14,
        Spi3Tx = 15,
        I2c1Rx = 16,
        I2c1Tx = 17,
        I2c2Rx = 18,
        I2c2Tx = 19,
        I2c3Rx = 20,
        I2c3Tx = 21,
        I2c4Rx = 22,
        I2c4Tx = 23,
        Usart1Rx = 24,
        Usart1Tx = 25,
        Usart2Rx = 26,
        Usart2Tx = 27,
        Usart3Rx = 28,
        Usart3Tx = 29,
        Uart4Rx = 30,
        Uart4Tx = 31,
        Uart5Rx = 32,
        Uart5Tx = 33,
        Lpuart1Rx = 34,
        Lpuart1Tx = 35,
        AdcReq2 = 36,
        AdcReq3 = 37,
        AdcReq4 = 38,
        AdcReq5 = 39,
        Quadspi = 40,
        Dac2Ch1 = 41,
        Tim1Ch1 = 42,
        Tim1Ch2 = 43,
        Tim1Ch3 = 44,
        Tim1Ch4 = 45,
        Tim1Up = 46,
        Tim1Trig = 47,
        Tim1Com = 48,
        Tim8Ch1 = 49,
        Tim8Ch2 = 50,
        Tim8Ch3 = 51,
        Tim8Ch4 = 52,
        Tim8Up = 53,
        Tim8Trig = 54,
        Tim8Com = 55,
        Tim2Ch1 = 56,
        Tim2Ch2 = 57,
        Tim2Ch3 = 58,
        Tim2Ch4 = 59,
        Tim2Up = 60,
        Tim3Ch1 = 61,
        Tim3Ch2 = 62,
        Tim3Ch3 = 63,
        Tim3Ch4 = 64,
        Tim3Up = 65,
        Tim3Trig = 66,
        Tim4Ch1 = 67,
        Tim4Ch2 = 68,
        Tim4Ch3 = 69,
        Tim4Ch4 = 70,
        Tim4Up = 71,
        Tim5Ch1 = 72,
        Tim5Ch2 = 73,
        Tim5Ch3 = 74,
        Tim5Ch4 = 75,
        Tim5Up = 76,
        Tim5Trig = 77,
        Tim15Ch1 = 78,
        Tim15Up = 79,
        Tim15Trig = 80,
        Tim15Com = 81,
        Tim16Ch1 = 82,
        Tim16Up = 83,
        Tim17Ch1 = 84,
        Tim17Up = 85,
        Tim20Ch1 = 86,
        Tim20Ch2 = 87,
        Tim20Ch3 = 88,
        Tim20Ch4 = 89,
        Tim20Up = 90,
        AesIn = 91,
        AesOut = 92,
        Tim20Trig = 93,
        Tim20Com = 94,
        Hrtim1Master = 95,
        Hrtim1TimerA = 96,
        Hrtim1TimerB = 97,
        Hrtim1TimerC = 98,
        Hrtim1TimerD = 99,
        Hrtim1TimerE = 100,
        Hrtim1TimerF = 101,
        Dac3Ch1 = 102,
        Dac3Ch2 = 103,
        Dac4Ch1 = 104,
        Dac4Ch2 = 105,
        Spi4Rx = 106,
        Spi4Tx = 107,
        Sai1A = 108,
        Sai1B = 109,
        FmacRead = 110,
        FmacWrite = 111,
        CordicRead = 112,
        CordicWrite = 113,
        Ucpd1Rx = 114,
        Ucpd1Tx = 115,
    };

    namespace Private
    {
        IO_STRUCT_WRAPPER(DMAMUX1, Dmamux1, DMAMUX_Channel_TypeDef);
    }

    using DmaMux1 = DmaMux<Private::Dmamux1, DmamuxSyncInput, DmamuxRequestInput>;

    // DMAMUX channels 0..7 are wired to DMA1 channels 1..8, channels 8..15 to DMA2 channels 1..8
    // (on devices with 6 DMA channels per controller mux channels 6, 7, 14, 15 are not used)
    using DmaMux1Channel1 = DmaMux1::Channel<0>;
    using DmaMux1Channel2 = DmaMux1::Channel<1>;
    using DmaMux1Channel3 = DmaMux1::Channel<2>;
    using DmaMux1Channel4 = DmaMux1::Channel<3>;
    using DmaMux1Channel5 = DmaMux1::Channel<4>;
    using DmaMux1Channel6 = DmaMux1::Channel<5>;
#if defined (DMA1_Channel7)
    using DmaMux1Channel7 = DmaMux1::Channel<6>;
#endif
#if defined (DMA1_Channel8)
    using DmaMux1Channel8 = DmaMux1::Channel<7>;
#endif
    using DmaMux2Channel1 = DmaMux1::Channel<8>;
    using DmaMux2Channel2 = DmaMux1::Channel<9>;
    using DmaMux2Channel3 = DmaMux1::Channel<10>;
    using DmaMux2Channel4 = DmaMux1::Channel<11>;
    using DmaMux2Channel5 = DmaMux1::Channel<12>;
    using DmaMux2Channel6 = DmaMux1::Channel<13>;
#if defined (DMA2_Channel7)
    using DmaMux2Channel7 = DmaMux1::Channel<14>;
#endif
#if defined (DMA2_Channel8)
    using DmaMux2Channel8 = DmaMux1::Channel<15>;
#endif
} // namespace Zhele

#endif //! ZHELE_PLATFORM_STM32_G4_DMAMUX_H
