/**
 * @file
 * Implement DMA request selection for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_DMAMUX_H
#define ZHELE_PLATFORM_STM32_H5_DMAMUX_H

#include <stm32h5xx.h>

#include "dma.h"

#include <cstdint>
#include <tuple>

namespace Zhele
{
    /**
     * @brief GPDMA hardware request lines
     */
    enum class DmamuxRequestInput : uint8_t
    {
        Adc1 = 0,
        Dac1Channel1 = 2,
        Dac1Channel2 = 3,
        Tim6Up = 4,
        Tim7Up = 5,
        Spi1Rx = 6,
        Spi1Tx = 7,
        Spi2Rx = 8,
        Spi2Tx = 9,
        Spi3Rx = 10,
        Spi3Tx = 11,
        I2c1Rx = 12,
        I2c1Tx = 13,
        I2c2Rx = 15,
        I2c2Tx = 16,
        Usart1Rx = 21,
        Usart1Tx = 22,
        Usart2Rx = 23,
        Usart2Tx = 24,
        Usart3Rx = 25,
        Usart3Tx = 26,
        LpUart1Rx = 45,
        LpUart1Tx = 46,
        Tim1Ch1 = 58,
        Tim1Ch2 = 59,
        Tim1Ch3 = 60,
        Tim1Ch4 = 61,
        Tim1Up = 62,
        Tim1Trig = 63,
        Tim1Com = 64,
        Tim2Ch1 = 72,
        Tim2Ch2 = 73,
        Tim2Ch3 = 74,
        Tim2Ch4 = 75,
        Tim2Up = 76,
        Tim3Ch1 = 77,
        Tim3Ch2 = 78,
        Tim3Ch3 = 79,
        Tim3Ch4 = 80,
        Tim3Up = 81,
        Tim3Trig = 82,
        LpTim1Ic1 = 102,
        LpTim1Ic2 = 103,
        LpTim1Ue = 104,
        LpTim2Ic1 = 105,
        LpTim2Ic2 = 106,
        LpTim2Ue = 107,
        HashIn = 111,
        I3c1Rx = 120,
        I3c1Tx = 121,
        I3c1Tc = 122,
        I3c1Rs = 123,
        I3c2Rx = 136,
        I3c2Tx = 137,
        I3c2Tc = 138,
        I3c2Rs = 139,
    };

    /**
     * @brief Facade over the per-channel request selection of one GPDMA instance
     *
     * @tparam _Channels Type list-free pack of the controller's channel types,
     *         indexed by channel number
     */
    template<typename... _Channels>
    class DmaMuxBase
    {
        using ChannelsTuple = std::tuple<_Channels...>;
    public:
        using RequestInput = DmamuxRequestInput;

        /**
         * @brief Request selection for one channel
         *
         * @tparam _ChannelNumber Channel number
         */
        template<unsigned _ChannelNumber>
        class Channel
        {
            using TargetChannel = std::tuple_element_t<_ChannelNumber, ChannelsTuple>;
        public:
            /**
             * @brief Set request input
             *
             * @param [in] input Request line
             *
             * @par Returns
             *  Nothing
             */
            static void SelectRequestInput(RequestInput input)
            {
                TargetChannel::SelectRequestInput(static_cast<uint8_t>(input));
            }

            /**
             * @brief Returns current request input
             *
             * @returns Current request input
             */
            static RequestInput GetRequestInput()
            {
                return static_cast<RequestInput>(TargetChannel::GetRequestInput());
            }
        };
    };

    using DmaMux1 = DmaMuxBase<Dma1Channel0, Dma1Channel1, Dma1Channel2, Dma1Channel3,
        Dma1Channel4, Dma1Channel5, Dma1Channel6, Dma1Channel7>;
    using DmaMux2 = DmaMuxBase<Dma2Channel0, Dma2Channel1, Dma2Channel2, Dma2Channel3,
        Dma2Channel4, Dma2Channel5, Dma2Channel6, Dma2Channel7>;

    using DmaMux1Channel0 = DmaMux1::Channel<0>;
    using DmaMux1Channel1 = DmaMux1::Channel<1>;
    using DmaMux1Channel2 = DmaMux1::Channel<2>;
    using DmaMux1Channel3 = DmaMux1::Channel<3>;
    using DmaMux1Channel4 = DmaMux1::Channel<4>;
    using DmaMux1Channel5 = DmaMux1::Channel<5>;
    using DmaMux1Channel6 = DmaMux1::Channel<6>;
    using DmaMux1Channel7 = DmaMux1::Channel<7>;

    using DmaMux2Channel0 = DmaMux2::Channel<0>;
    using DmaMux2Channel1 = DmaMux2::Channel<1>;
    using DmaMux2Channel2 = DmaMux2::Channel<2>;
    using DmaMux2Channel3 = DmaMux2::Channel<3>;
    using DmaMux2Channel4 = DmaMux2::Channel<4>;
    using DmaMux2Channel5 = DmaMux2::Channel<5>;
    using DmaMux2Channel6 = DmaMux2::Channel<6>;
    using DmaMux2Channel7 = DmaMux2::Channel<7>;
} // namespace Zhele

#endif //! ZHELE_PLATFORM_STM32_H5_DMAMUX_H
