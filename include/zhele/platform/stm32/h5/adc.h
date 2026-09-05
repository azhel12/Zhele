/**
 * @file
 * Implements ADC for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_ADC_H
#define ZHELE_PLATFORM_STM32_H5_ADC_H

#include <stm32h5xx.h>

#include "../common/ioreg.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace Zhele
{
    using AdcCallbackType = std::add_pointer_t<void(uint16_t* data, uint32_t count)>;

    namespace Private
    {
        /**
         * @brief Values shared by every ADC instance
         */
        class AdcCommon
        {
        public:
            using DataT = uint16_t;

            /// Nominal reference voltage, in units of 0.1 mV (3.3 V)
            static const unsigned VRefNominal = 33000;

            /// Maximum number of conversions in one regular sequence
            static const unsigned MaxRegular = 16;

            /// Internal channels (RM0492 section 19.4.4)
            static const uint8_t VBatChannel = 2;        ///< VBAT / 4
            static const uint8_t VCoreChannel = 6;       ///< VDDCORE
            static const uint8_t TempSensorChannel = 16; ///< VSENSE
            static const uint8_t ReferenceChannel = 17;  ///< VREFINT

            /// Number of external + internal channels
            static const uint8_t ChannelCount = 20;

            /// Factory calibration data in system memory (DS14053 tables 22 and 69)
            static constexpr uint32_t VRefIntCalAddress = 0x08FFF810;      ///< VREFINT raw value at 30 C, VDDA = 3.3 V
            static constexpr uint32_t TempSensorCal1Address = 0x08FFF814;  ///< VSENSE raw value at 30 C
            static constexpr uint32_t TempSensorCal2Address = 0x08FFF818;  ///< VSENSE raw value at 130 C
            static constexpr int32_t TempSensorCal1Temp = 30;              ///< Temperature of the first calibration point
            static constexpr int32_t TempSensorCal2Temp = 130;             ///< Temperature of the second calibration point

            /// Conversion result resolution
            enum class Resolution
            {
                Bit12 = 0b00 << ADC_CFGR_RES_Pos, ///< 12 bits (default)
                Bit10 = 0b01 << ADC_CFGR_RES_Pos, ///< 10 bits
                Bit8 = 0b10 << ADC_CFGR_RES_Pos,  ///< 8 bits
                Bit6 = 0b11 << ADC_CFGR_RES_Pos   ///< 6 bits
            };

            /// Sampling time, in ADC clock cycles
            enum class SampleTime
            {
                Cycles2_5 = 0b000,
                Cycles6_5 = 0b001,
                Cycles12_5 = 0b010,
                Cycles24_5 = 0b011,
                Cycles47_5 = 0b100,
                Cycles92_5 = 0b101,
                Cycles247_5 = 0b110,
                Cycles640_5 = 0b111
            };

            /// Kernel clock prescaler (ADC_CCR.PRESC)
            enum class AdcDivider
            {
                Div1 = 0b0000,
                Div2 = 0b0001,
                Div4 = 0b0010,
                Div6 = 0b0011,
                Div8 = 0b0100,
                Div10 = 0b0101,
                Div12 = 0b0110,
                Div16 = 0b0111,
                Div32 = 0b1000,
                Div64 = 0b1001,
                Div128 = 0b1010,
                Div256 = 0b1011
            };

            /// Error codes reported by GetError()
            enum class AdcError
            {
                NoError,        ///< No error
                Overflow,       ///< Overrun
                TransferError,  ///< DMA transfer error
                NotReady        ///< ADC not ready
            };
        };

        /**
         * @brief Implements one ADC
         *
         * @tparam _Regs ADC registers
         * @tparam _CommonRegs ADCx_COMMON registers
         * @tparam _ClockCtrl ADC clock control
         * @tparam _IRQNumber ADC IRQ number
         * @tparam _DmaChannel DMA channel used for regular conversions (or void)
         */
        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _DmaChannel>
        class Adc : public AdcCommon
        {
            static AdcCallbackType _callback;
            static AdcError _error;

        public:
            /**
             * @brief Power up, calibrate and enable the converter
             *
             * @details
             * Leaves deep power-down, turns on the internal regulator, runs a
             * single-ended calibration and enables the ADC. Blocks until ready.
             *
             * @tparam divider Kernel clock prescaler
             *
             * @par Returns
             *  Nothing
             */
            template<AdcDivider divider = AdcDivider::Div4>
            static void Init()
            {
                _ClockCtrl::Enable();

                _CommonRegs()->CCR = (_CommonRegs()->CCR & ~ADC_CCR_PRESC_Msk)
                    | (static_cast<uint32_t>(divider) << ADC_CCR_PRESC_Pos);

                // Leave deep power-down and start the voltage regulator
                _Regs()->CR &= ~ADC_CR_DEEPPWD;
                _Regs()->CR |= ADC_CR_ADVREGEN;

                // The regulator needs ~10 us; at 250 MHz that is a few thousand cycles
                for (volatile uint32_t i = 0; i < 4000; ++i)
                    ;

                Calibrate();

                _Regs()->ISR = ADC_ISR_ADRDY;
                _Regs()->CR |= ADC_CR_ADEN;
                while ((_Regs()->ISR & ADC_ISR_ADRDY) == 0)
                    ;
            }

            /**
             * @brief Run single-ended calibration
             *
             * @details
             * Must be done with the ADC disabled but the regulator running.
             *
             * @par Returns
             *  Nothing
             */
            static void Calibrate()
            {
                Disable();

                _Regs()->CR &= ~ADC_CR_ADCALDIF;
                _Regs()->CR |= ADC_CR_ADCAL;
                while ((_Regs()->CR & ADC_CR_ADCAL) != 0)
                    ;
            }

            /**
             * @brief Disable the converter
             *
             * @par Returns
             *  Nothing
             */
            static void Disable()
            {
                if ((_Regs()->CR & ADC_CR_ADEN) != 0)
                {
                    _Regs()->CR |= ADC_CR_ADDIS;
                    while ((_Regs()->CR & ADC_CR_ADEN) != 0)
                        ;
                }
            }

            /**
             * @brief Set conversion resolution
             *
             * @param [in] resolution Resolution
             *
             * @par Returns
             *  Nothing
             */
            static void SetResolution(Resolution resolution)
            {
                _Regs()->CFGR = (_Regs()->CFGR & ~ADC_CFGR_RES_Msk) | static_cast<uint32_t>(resolution);
            }

            /**
             * @brief Set sampling time for one channel
             *
             * @param [in] channel Channel number
             * @param [in] sampleTime Sampling time
             *
             * @par Returns
             *  Nothing
             */
            static void SetSampleTime(uint8_t channel, SampleTime sampleTime)
            {
                auto value = static_cast<uint32_t>(sampleTime);

                if (channel < 10)
                {
                    uint32_t shift = channel * 3;
                    _Regs()->SMPR1 = (_Regs()->SMPR1 & ~(0x7u << shift)) | (value << shift);
                }
                else
                {
                    uint32_t shift = (channel - 10) * 3;
                    _Regs()->SMPR2 = (_Regs()->SMPR2 & ~(0x7u << shift)) | (value << shift);
                }
            }

            /**
             * @brief Enable the internal reference voltage channel
             *
             * @par Returns
             *  Nothing
             */
            static void EnableReferenceChannel()
            {
                _CommonRegs()->CCR |= ADC_CCR_VREFEN;
            }

            /**
             * @brief Enable the internal temperature sensor channel
             *
             * @par Returns
             *  Nothing
             */
            static void EnableTempSensorChannel()
            {
                _CommonRegs()->CCR |= ADC_CCR_TSEN;
            }

            /**
             * @brief Enable the VBAT/4 monitoring channel
             *
             * @par Returns
             *  Nothing
             */
            static void EnableVBatChannel()
            {
                _CommonRegs()->CCR |= ADC_CCR_VBATEN;
            }

            /**
             * @brief Configure a pin as an analog ADC input
             *
             * @tparam Pin Target pin
             *
             * @par Returns
             *  Nothing
             */
            template<typename Pin>
            static void SelectChannelPin()
            {
                Pin::Port::Enable();
                Pin::template SetConfiguration<Pin::Port::Configuration::Analog>();
            }

            /**
             * @brief Convert one channel and return the result
             *
             * @details
             * Programs a one-conversion sequence, starts it and waits.
             *
             * @param [in] channel Channel number
             *
             * @returns Conversion result
             */
            static DataT SingleConversion(uint8_t channel)
            {
                StopRegular();

                _Regs()->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
                _Regs()->SQR1 = static_cast<uint32_t>(channel) << ADC_SQR1_SQ1_Pos;

                _Regs()->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
                _Regs()->CR |= ADC_CR_ADSTART;

                while ((_Regs()->ISR & ADC_ISR_EOC) == 0)
                    ;

                return static_cast<DataT>(_Regs()->DR);
            }

            /**
             * @brief Set the callback invoked when a regular sequence completes
             *
             * @param [in] callback Callback
             *
             * @par Returns
             *  Nothing
             */
            static void SetRegularCallback(AdcCallbackType callback)
            {
                _callback = callback;
            }

            /**
             * @brief Returns true when a new regular sequence may be started
             *
             * @retval true Ready
             * @retval false Conversion in progress
             */
            static bool RegularReady()
            {
                return (_Regs()->CR & ADC_CR_ADSTART) == 0;
            }

            /**
             * @brief Start a continuous, DMA-fed regular sequence
             *
             * @details
             * The sequence is converted in continuous mode and the results are
             * written into @p dataBuffer by DMA; the callback (if any) fires when
             * the buffer is full. Requires a DMA channel bound at instantiation
             * and its request line pointed at this ADC.
             *
             * @param [in] channels Channel numbers, in conversion order
             * @param [in] channelsCount Number of channels (1..16)
             * @param [out] dataBuffer Destination buffer
             * @param [in] scanCount Number of complete sequences to store
             *
             * @retval true Started
             * @retval false ADC busy or arguments invalid
             */
            static bool StartRegular(const uint8_t* channels, uint8_t channelsCount, DataT* dataBuffer, uint16_t scanCount)
            {
                static_assert(!std::is_same_v<_DmaChannel, void>, "StartRegular needs a DMA channel");

                if (channelsCount == 0 || channelsCount > MaxRegular)
                {
                    _error = AdcError::NotReady;
                    return false;
                }

                if (!RegularReady())
                {
                    _error = AdcError::NotReady;
                    return false;
                }

                SetSequence(channels, channelsCount);

                _error = AdcError::NoError;

                _DmaChannel::ClearTransferComplete();
                _DmaChannel::SetTransferCallback(&DmaHandler);
                _DmaChannel::Transfer(
                    _DmaChannel::Periph2Mem | _DmaChannel::MemIncrement | _DmaChannel::PSize16Bits | _DmaChannel::MSize16Bits,
                    dataBuffer, &_Regs()->DR, static_cast<uint32_t>(channelsCount) * scanCount);

                // DMACFG = 1: circular, so the ADC keeps feeding the channel
                _Regs()->CFGR |= (ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
                _Regs()->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
                _Regs()->CR |= ADC_CR_ADSTART;

                return true;
            }

            /**
             * @brief Start a continuous, DMA-fed regular sequence
             *
             * @param [in] channels Channel numbers, in conversion order
             * @param [out] dataBuffer Destination buffer
             * @param [in] scanCount Number of complete sequences to store
             *
             * @retval true Started
             * @retval false ADC busy or arguments invalid
             */
            static bool StartRegular(std::initializer_list<uint8_t> channels, DataT* dataBuffer, uint16_t scanCount)
            {
                return StartRegular(channels.begin(), static_cast<uint8_t>(channels.size()), dataBuffer, scanCount);
            }

            /**
             * @brief Stop the regular sequence
             *
             * @par Returns
             *  Nothing
             */
            static void StopRegular()
            {
                if ((_Regs()->CR & ADC_CR_ADSTART) != 0)
                {
                    _Regs()->CR |= ADC_CR_ADSTP;
                    while ((_Regs()->CR & ADC_CR_ADSTP) != 0)
                        ;
                }

                _Regs()->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
            }

            /**
             * @brief Returns the last error
             *
             * @returns Error code
             */
            static AdcError GetError()
            {
                if ((_Regs()->ISR & ADC_ISR_OVR) != 0)
                    return AdcError::Overflow;

                return _error;
            }

            /**
             * @brief Convert a raw result to units of 0.1 mV
             *
             * @details
             * Uses the factory VREFINT calibration value, so the result is
             * independent of the actual VREF+ only if the reference channel has
             * been measured; otherwise the nominal 3.3 V is assumed.
             *
             * @param [in] value Raw conversion result
             *
             * @returns Voltage, in units of 0.1 mV
             */
            static unsigned ToVolts(DataT value)
            {
                return static_cast<unsigned>(value) * VRefNominal / 4095u;
            }

            /**
             * @brief Read the internal temperature sensor, in tenths of a degree
             *
             * @details
             * Uses the factory calibration points stored in system memory
             * (TS_CAL1 at 30 degrees, TS_CAL2 at 130 degrees), both acquired at
             * VDDA = 3.3 V.
             *
             * @returns Temperature, in 0.1 degrees Celsius
             */
            static int16_t ReadTemperature()
            {
                // Calibration values are measured at VDDA = 3.0 V
                auto cal1 = *reinterpret_cast<const volatile uint16_t*>(TempSensorCal1Address);
                auto cal2 = *reinterpret_cast<const volatile uint16_t*>(TempSensorCal2Address);

                EnableTempSensorChannel();
                SetSampleTime(TempSensorChannel, SampleTime::Cycles640_5);

                int32_t raw = SingleConversion(TempSensorChannel);

                if (cal2 == cal1)
                    return 0;

                return static_cast<int16_t>(
                    (raw - static_cast<int32_t>(cal1)) * (TempSensorCal2Temp - TempSensorCal1Temp) * 10
                        / (static_cast<int32_t>(cal2) - static_cast<int32_t>(cal1))
                    + TempSensorCal1Temp * 10);
            }

            /**
             * @brief DMA transfer complete handler
             *
             * @param [in] data Data buffer
             * @param [in] size Buffer size
             * @param [in] success Transfer result
             *
             * @par Returns
             *  Nothing
             */
            static void DmaHandler(void* data, unsigned size, bool success)
            {
                if (!success)
                    _error = AdcError::TransferError;

                if (_callback)
                    _callback(reinterpret_cast<DataT*>(data), size);
            }

            /**
             * @brief ADC IRQ handler
             *
             * @par Returns
             *  Nothing
             */
            static void IrqHandler()
            {
                if ((_Regs()->ISR & ADC_ISR_OVR) != 0)
                {
                    _error = AdcError::Overflow;
                    _Regs()->ISR = ADC_ISR_OVR;
                }
            }

        private:
            /**
             * @brief Program the regular sequence
             */
            static void SetSequence(const uint8_t* channels, uint8_t count)
            {
                uint32_t sqr[4] = {static_cast<uint32_t>(count - 1), 0, 0, 0};

                for (uint8_t i = 0; i < count; ++i)
                {
                    // SQR1 holds ranks 1..4 starting at bit 6, SQR2..SQR4 hold five each
                    unsigned rank = i;
                    unsigned reg = (rank + 1) / 5;
                    unsigned shift = reg == 0
                        ? 6 + rank * 6
                        : ((rank + 1) % 5) * 6;

                    sqr[reg] |= static_cast<uint32_t>(channels[i]) << shift;
                }

                _Regs()->SQR1 = sqr[0];
                _Regs()->SQR2 = sqr[1];
                _Regs()->SQR3 = sqr[2];
                _Regs()->SQR4 = sqr[3];
            }
        };

        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _DmaChannel>
        AdcCallbackType Adc<_Regs, _CommonRegs, _ClockCtrl, _IRQNumber, _DmaChannel>::_callback = nullptr;

        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _DmaChannel>
        AdcCommon::AdcError Adc<_Regs, _CommonRegs, _ClockCtrl, _IRQNumber, _DmaChannel>::_error = AdcCommon::AdcError::NoError;

        IO_STRUCT_WRAPPER(ADC1, Adc1Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC12_COMMON, Adc1CommonRegs, ADC_Common_TypeDef);
    }

    /**
     * @brief Pin to ADC channel mapping for STM32H503
     *
     * @details
     * Not every pin exists in every package; see DS14053 table 10.
     */
    namespace AdcChannels
    {
        template<typename Pin> struct ChannelOf;

        template<> struct ChannelOf<IO::Pa0> { static constexpr uint8_t value = 0; };
        template<> struct ChannelOf<IO::Pa1> { static constexpr uint8_t value = 1; };
        template<> struct ChannelOf<IO::Pa6> { static constexpr uint8_t value = 3; };
        template<> struct ChannelOf<IO::Pc4> { static constexpr uint8_t value = 4; };
        template<> struct ChannelOf<IO::Pb1> { static constexpr uint8_t value = 5; };
        template<> struct ChannelOf<IO::Pa7> { static constexpr uint8_t value = 7; };
        template<> struct ChannelOf<IO::Pc5> { static constexpr uint8_t value = 8; };
        template<> struct ChannelOf<IO::Pb0> { static constexpr uint8_t value = 9; };
        template<> struct ChannelOf<IO::Pc0> { static constexpr uint8_t value = 10; };
        template<> struct ChannelOf<IO::Pc1> { static constexpr uint8_t value = 11; };
        template<> struct ChannelOf<IO::Pc2> { static constexpr uint8_t value = 12; };
        template<> struct ChannelOf<IO::Pc3> { static constexpr uint8_t value = 13; };
        template<> struct ChannelOf<IO::Pa2> { static constexpr uint8_t value = 14; };
        template<> struct ChannelOf<IO::Pa3> { static constexpr uint8_t value = 15; };
        template<> struct ChannelOf<IO::Pa4> { static constexpr uint8_t value = 18; };
        template<> struct ChannelOf<IO::Pa5> { static constexpr uint8_t value = 19; };

        /// ADC channel number of a pin
        template<typename Pin>
        constexpr uint8_t Channel = ChannelOf<Pin>::value;
    }

    template<typename _DmaChannel = void>
    using Adc1 = Private::Adc<Private::Adc1Regs, Private::Adc1CommonRegs, Clock::Adc1Clock, ADC1_IRQn, _DmaChannel>;
    using Adc1NoDma = Adc1<>;
}

#endif //! ZHELE_PLATFORM_STM32_H5_ADC_H
