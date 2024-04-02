/**
 * @file
 * Implement DMAMUX module for stm32g0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_DMAMUX_H
#define ZHELE_DMAMUX_H

#include <stm32g0xx.h>
#include "../common/dmamux.h"

namespace Zhele
{
    enum DmamuxSyncInput
    {
        Exti0 = 0,
        Exti1 = 1,
        Exti2 = 2,
        Exti3 = 3,
        Exti4 = 4,
        Exti5 = 5,
        Exti6 = 6,
        Exti7 = 7,
        Exti8 = 8,
        Exti9 = 9,
        Exti10 = 10,
        Exti11 = 11,
        Exti12 = 12,
        Exti13 = 13,
        Exti14 = 14,
        Exti15 = 15,
        Evt0 = 16,
        Evt1 = 17,
        Evt2 = 18,
        Evt3 = 19,
        Tim14OC = 22,
    };

    enum DmamuxRequestInput
    {
        RequestGen0 = 1,
        RequestGen1 = 2,
        RequestGen2 = 3,
        RequestGen3 = 4,
        Adc = 5,
        I2c1Rx = 10,
        I2c1Tx = 11,
        I2c2Rx = 12,
        I2c2Tx = 13,
        Spi1Rx = 16,
        Spi1Tx = 17,
        Spi2Rx = 18,
        Spi2Tx = 19,
        Tim1Ch1 = 20,
        Tim1Ch2 = 21,
        Tim1Ch3 = 22,
        Tim1Ch4 = 23,
        Tim1TrigCom = 24,
        Tim1Up = 25,
        Tim3Ch1 = 32,
        Tim3Ch2 = 33,
        Tim3Ch3 = 34,
        Tim3Ch4 = 35,
        Tim3Trig = 36,
        Tim3Up = 37,
        Tim6Up = 38,
        Tim7Up = 39,
        Tim15Ch1 = 40,
        Tim15Ch2 = 41,
        Tim15TrigCom = 42,
        Tim15Up = 43,
        Tim16Ch1 = 44,
        Tim16Com = 45,
        Tim16Up = 46,
        Tim17Ch1 = 47,
        Tim17Com = 48,
        Tim17Up = 49,
        Usart1Rx = 50,
        Usart1Tx = 51,
        Usart2Rx = 52,
        Usart2Tx = 53,
        Usart3Rx = 54,
        Usart3Tx = 55,
        Usart4Rx = 56,
        Usart4Tx = 57,
        I2c3Rx = 62,
        I2c3Tx = 63,
        Spi3Rx = 66,
        Spi3Tx = 67,
        Tim4Ch1 = 68,
        Tim4Ch2 = 69,
        Tim4Ch3 = 70,
        Tim4Ch4 = 71,
        Tim4Trig = 72,
        Tim4Up = 73,
        Usart5Rx = 74,
        Usart5Tx = 75,
        Usart6Rx = 76,
        Usart6Tx = 77,
    };

    namespace Private
    {
        IO_STRUCT_WRAPPER(DMAMUX1, Dmamux1, DMAMUX_Channel_TypeDef);
    }

    using DmaMux1 = DmaMux<Private::Dmamux1, DmamuxSyncInput, DmamuxRequestInput>;
    using DmaMux1Channel1 = DmaMux1::Channel<0>;
    using DmaMux1Channel2 = DmaMux1::Channel<1>;
    using DmaMux1Channel3 = DmaMux1::Channel<2>;
    using DmaMux1Channel4 = DmaMux1::Channel<3>;
    using DmaMux1Channel5 = DmaMux1::Channel<4>;

} // namespace Zhele

#endif //! ZHELE_DMAMUX_H