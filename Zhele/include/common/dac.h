
/**
 * @file
 * @brief Implements DAC
 * @date 2022
 * @author Aleksei Zhelonkin
 * @license FreeBSD
 */
#ifndef ZHELE_DAC_COMMON_H
#define ZHELE_DAC_COMMON_H

#include <cstdint>

#include <clock.h>

namespace Zhele
{
    namespace Private
    {
        /**
         * @brief Implementd Digital-to-Analog converter
         * 
         * @tparam _Regs Registers
         * @tparam _ClockCtrl Clock control
         * @tparam _Channel Channel
         */
        template <typename _Regs, typename _ClockCtrl, uint8_t _Channel>
        class DacBase
        {
            static const uint8_t ChannelOffset = 16;
            static_assert(_Channel == 0 || _Channel == 1, "Channel can be only 0 or 1");

            /// Wave type
            enum WaveGeneration : uint8_t
            {
                Disabled = 0b00, ///< Wave generation disabled
                Noise = 0b01, ///< Noise generation
                Triangle = 0b10, ///< Triangle generation
            };

        public:
            /// Wave amplitude
            enum class WaveAmplitude : uint8_t
            {
                Equal1 = 0x00, ///< Amplitude equal to 1
                Equal3 = 0x01, ///< Amplitude equal to 3
                Equal7 = 0x02, ///< Amplitude equal to 7
                Equal15 = 0x03, ///< Amplitude equal to 15
                Equal31 = 0x04, ///< Amplitude equal to 31
                Equal63 = 0x05, ///< Amplitude equal to 63
                Equal127 = 0x06, ///< Amplitude equal to 127
                Equal255 = 0x07, ///< Amplitude equal to 255
                Equal511 = 0x08, ///< Amplitude equal to 511
                Equal1023 = 0x09, ///< Amplitude equal to 1023
                Equal2047 = 0x0a, ///< Amplitude equal to 2047
                Equal4095 = 0x0b, ///< Amplitude equal to 4095
            };

            /**
             * @brief Init DAC channel
             * 
             * @par Returns
             *  Nothing
             */
            static void Init();
            
            /**
             * @brief Init DAC channel with trigger
             * 
             * @tparam Trigger Trigger type
             * 
             * @param trigger Trigger
             * 
             * @par Returns
             *  Nothing
             */
            template <typename Trigger>
            static void Init(Trigger trigger);

            /**
             * @brief Enable channel
             * 
             * @par Returns
             *  Nothing
             */
            static void Enable();            

            /**
             * @brief Disable channel
             * 
             * @par Returns
             *  Nothing
             */
            static void Disable();

            /**
             * @brief Enable output buffer
             * 
             * @par Returns
             *  Nothing
             */
            static void EnableBuffer();

            /**
             * @brief Disable output buffer
             * 
             * @par Returns
             *  Nothing
             */
            static void DisableBuffer();

            /**
             * @brief Enable noise wave generation
             * 
             * @param amplitude Amplitude value
             * 
             * @par Returns
             *  Nothing
             */
            static void EnableNoiseWaveGeneration(WaveAmplitude amplitude);

            /**
             * @brief Enable noise wave generation
             * 
             * @param amplitude Amplitude value
             * 
             * @par Returns
             *  Nothing
             */
            static void EnableTriangleWaveGeneration(WaveAmplitude amplitude);

            /**
             * @brief Disable wave generation
             * 
             * @par Returns
             *  Nothing
             */
            static void DisableWaveGeneration();

            /**
             * @brief Write 8-bit data
             * 
             * @param data Data
             * 
             * @par Returns
             *  Nothing
             */
            static void WriteU8(uint8_t data);

            /**
             * @brief Write 12-bit right-aligned data
             * 
             * @param data Data
             * 
             * @par Returns
             *  Nothing
             */
            static void Write(uint16_t data);

            /**
             * @brief Write 12-bit left-aligned data
             * 
             * @param data Data
             * 
             * @par Returns
             *  Nothing
             */
            static void WriteLeftAligned(uint16_t data);

            /**
             * @brief Cause software trigger
             * 
             * @par Returns
             *  Nothing
             */
            static void CauseSoftwareTrigger();
        };
#if defined (DAC1)
        IO_STRUCT_WRAPPER(DAC1, Dac1Regs, DAC_TypeDef);
#endif

    } // namespace Private
#if defined (DAC1)
    using Dac1Channel1 = Private::DacBase<Private::Dac1Regs, Clock::DacClock, 0>;
    using Dac1Channel2 = Private::DacBase<Private::Dac1Regs, Clock::DacClock, 1>;
#endif

} // namespace Zhele

#include "impl/dac.h"

#endif //! ZHELE_DAC_COMMON_H