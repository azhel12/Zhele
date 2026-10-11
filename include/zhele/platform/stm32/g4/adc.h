/**
 * @file
 * Implements ADC for stm32g4 series
 *
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 *
 * @details
 * Same ADC IP as H5 (ADEN/ADCAL, CFGR/SQRx/SMPRx), so the API follows h5/adc.h.
 * Differences: up to five instances in two pairs (ADC12, ADC345) and internal
 * channels that depend on the instance (RM0440 section 21.4.11), so each instance
 * gets its own pin map.
 *
 * Not implemented: injected channels, external triggers, oversampling,
 * differential mode, dual (simultaneous) mode.
 */

#ifndef ZHELE_PLATFORM_STM32_G4_ADC_H
#define ZHELE_PLATFORM_STM32_G4_ADC_H

#include <stm32g4xx.h>

#include "../common/ioreg.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include <zhele/delay.h>
#include <zhele/pinlist.h>

#include <array>
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

            /// Marks an internal channel that is not wired to the instance
            static const uint8_t NoChannel = 0xFF;

            /// Number of external + internal channels
            static const uint8_t ChannelCount = 19;

            /// Factory calibration data in system memory (DS12288 tables 8 and 9), acquired at VDDA = 3.0 V
            static constexpr uint32_t VRefIntCalAddress = 0x1FFF75AA;      ///< VREFINT raw value at 30 C
            static constexpr uint32_t TempSensorCal1Address = 0x1FFF75A8;  ///< VSENSE raw value at 30 C
            static constexpr uint32_t TempSensorCal2Address = 0x1FFF75CA;  ///< VSENSE raw value at 130 C
            static constexpr int32_t TempSensorCal1Temp = 30;              ///< Temperature of the first calibration point
            static constexpr int32_t TempSensorCal2Temp = 130;             ///< Temperature of the second calibration point
            static constexpr unsigned CalibrationVdda = 3000;              ///< VDDA of the calibration, mV

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

            /**
             * @brief Synchronous clock mode (ADC_CCR.CKMODE)
             *
             * @details
             * The asynchronous kernel clock is not used: its RCC selector (CCIPR.ADCxxSEL)
             * resets to "no clock". ADC clock must not exceed 60 MHz, so at 170 MHz HCLK only Div4 fits.
             */
            enum class AdcDivider
            {
                Div1 = 0b01, ///< HCLK / 1 (AHB prescaler must be 1)
                Div2 = 0b10, ///< HCLK / 2
                Div4 = 0b11  ///< HCLK / 4
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
         * @brief Returns ADC channel of a pin from a pin map
         */
        template<typename _Pins, typename Pin>
        constexpr uint8_t AdcPinChannel()
        {
            constexpr int index = _Pins::io_pins::template IndexOf<Pin>;
            static_assert(index >= 0, "Pin is not an input of this ADC");
            return _Pins::channels[index];
        }

        /**
         * @brief Implements one ADC
         *
         * @tparam _Regs ADC registers
         * @tparam _CommonRegs ADC12_COMMON / ADC345_COMMON registers (shared by the pair)
         * @tparam _ClockCtrl ADC clock control
         * @tparam _IRQNumber ADC IRQ number
         * @tparam _Pins Pin map: `io_pins` list, `channels` array (channel of each pin) and internal channels
         * @tparam _DmaChannel DMA channel used for regular conversions (or void)
         */
        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _Pins, typename _DmaChannel>
        class Adc : public AdcCommon
        {
            static AdcCallbackType _callback;
            static AdcError _error;
            static uint16_t _vdda;

        public:
            /// Internal channels of this instance (NoChannel if not wired)
            static const uint8_t TempSensorChannel = _Pins::TempSensorChannel; ///< VSENSE
            static const uint8_t VBatChannel = _Pins::VBatChannel;             ///< VBAT / 3
            static const uint8_t ReferenceChannel = _Pins::ReferenceChannel;   ///< VREFINT

            /// ADC channel number of a pin
            template<typename Pin>
            static constexpr uint8_t Channel = AdcPinChannel<_Pins, Pin>();

            /**
             * @brief Power up, calibrate and enable the converter
             *
             * @details
             * Leaves deep power-down, turns on the internal regulator, runs a
             * single-ended calibration and enables the ADC. Blocks until ready.
             * Does nothing if the ADC is already enabled.
             *
             * @tparam divider Synchronous clock divider (shared by the pair, set by the first Init())
             *
             * @par Returns
             *  Nothing
             */
            template<AdcDivider divider = AdcDivider::Div4>
            static void Init()
            {
                // ADCAL/ADVREGEN must not be touched while ADEN = 1
                if ((_Regs()->CR & ADC_CR_ADEN) != 0)
                    return;

                _ClockCtrl::Enable();

                // CKMODE may be written only while both ADCs of the pair are disabled
                if ((_CommonRegs()->CCR & ADC_CCR_CKMODE) == 0)
                    _CommonRegs()->CCR |= static_cast<uint32_t>(divider) << ADC_CCR_CKMODE_Pos;

                // Leave deep power-down and start the voltage regulator
                _Regs()->CR &= ~ADC_CR_DEEPPWD;
                _Regs()->CR |= ADC_CR_ADVREGEN;
                delay_us<20>(); // tADCVREG_STUP

                Calibrate();

                _Regs()->ISR = ADC_ISR_ADRDY;
                _Regs()->CR |= ADC_CR_ADEN;
                // ADEN set less than 4 ADC cycles after the calibration is silently reset: repeat until ADRDY
                while ((_Regs()->ISR & ADC_ISR_ADRDY) == 0)
                {
                    if ((_Regs()->CR & ADC_CR_ADEN) == 0)
                        _Regs()->CR |= ADC_CR_ADEN;
                }
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
             * @brief Enable the internal reference voltage channel (shared by the pair)
             *
             * @par Returns
             *  Nothing
             */
            static void EnableReferenceChannel()
            {
                static_assert(ReferenceChannel != NoChannel, "VREFINT is not connected to this ADC");
                _CommonRegs()->CCR |= ADC_CCR_VREFEN;
            }

            /**
             * @brief Disable the internal reference voltage channel
             *
             * @par Returns
             *  Nothing
             */
            static void DisableReferenceChannel()
            {
                _CommonRegs()->CCR &= ~ADC_CCR_VREFEN;
            }

            /**
             * @brief Enable the internal temperature sensor channel (shared by the pair)
             *
             * @par Returns
             *  Nothing
             */
            static void EnableTempSensorChannel()
            {
                static_assert(TempSensorChannel != NoChannel, "Temperature sensor is not connected to this ADC");
                _CommonRegs()->CCR |= ADC_CCR_VSENSESEL;
            }

            /**
             * @brief Disable the internal temperature sensor channel
             *
             * @par Returns
             *  Nothing
             */
            static void DisableTempSensorChannel()
            {
                _CommonRegs()->CCR &= ~ADC_CCR_VSENSESEL;
            }

            /**
             * @brief Enable the VBAT/3 monitoring channel (shared by the pair)
             *
             * @par Returns
             *  Nothing
             */
            static void EnableVBatChannel()
            {
                static_assert(VBatChannel != NoChannel, "VBAT is not connected to this ADC");
                _CommonRegs()->CCR |= ADC_CCR_VBATSEL;
            }

            /**
             * @brief Disable the VBAT/3 monitoring channel
             *
             * @par Returns
             *  Nothing
             */
            static void DisableVBatChannel()
            {
                _CommonRegs()->CCR &= ~ADC_CCR_VBATSEL;
            }

            /**
             * @brief Configure a pin as an analog ADC input
             *
             * @details
             * Init() does not touch pins: a pin map lists pins shared with other peripherals.
             *
             * @tparam Pin Target pin
             *
             * @par Returns
             *  Nothing
             */
            template<typename Pin>
            static void SelectChannelPin()
            {
                static_assert(_Pins::io_pins::template IndexOf<Pin> >= 0, "Pin is not an input of this ADC");
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

                _Regs()->SQR1 = static_cast<uint32_t>(channel) << ADC_SQR1_SQ1_Pos;

                _Regs()->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
                _Regs()->CR |= ADC_CR_ADSTART;

                while ((_Regs()->ISR & ADC_ISR_EOC) == 0)
                    ;

                return static_cast<DataT>(_Regs()->DR);
            }

            /**
             * @brief Convert the channel of a pin (switches the pin to analog mode first)
             *
             * @tparam Pin Input pin
             *
             * @returns Conversion result
             */
            template<typename Pin>
            static DataT SingleConversion()
            {
                SelectChannelPin<Pin>();
                return SingleConversion(Channel<Pin>);
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
             * written into @p dataBuffer by circular DMA; the callback (if any) fires when
             * the buffer is full. Requires a DMA channel bound at instantiation
             * and its DMAMUX request routed to this ADC (AdcReqN, see dmamux.h).
             * Input pins must be configured with SelectChannelPin().
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

                if (channelsCount == 0 || channelsCount > MaxRegular || scanCount == 0)
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
                    _DmaChannel::Periph2Mem | _DmaChannel::MemIncrement | _DmaChannel::Circular
                        | _DmaChannel::PSize16Bits | _DmaChannel::MSize16Bits,
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

                if ((_Regs()->CFGR & ADC_CFGR_DMAEN) != 0)
                {
                    _Regs()->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
                    if constexpr (!std::is_same_v<_DmaChannel, void>)
                        _DmaChannel::Disable();
                }
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
             * @brief Measure VDDA through VREFINT and its factory calibration
             *
             * @details
             * The result is cached and used by ToVolts() and ReadTemperature()
             * (nominal 3.3 V until the first measurement).
             *
             * @returns VDDA, mV
             */
            static uint16_t MeasureVdda()
            {
                auto cal = *reinterpret_cast<const volatile uint16_t*>(VRefIntCalAddress);

                EnableReferenceChannel();
                delay_us<12>(); // tSTART_VREFINT
                SetSampleTime(ReferenceChannel, SampleTime::Cycles640_5);

                uint32_t raw = SingleConversion(ReferenceChannel);

                if (raw != 0)
                    _vdda = static_cast<uint16_t>(CalibrationVdda * cal / raw);

                return _vdda;
            }

            /**
             * @brief Convert a raw result to units of 0.1 mV
             *
             * @details
             * Uses VDDA measured by MeasureVdda(), nominal 3.3 V otherwise.
             *
             * @param [in] value Raw conversion result
             *
             * @returns Voltage, in units of 0.1 mV
             */
            static unsigned ToVolts(DataT value)
            {
                return static_cast<unsigned>(value) * _vdda * 10u / 4095u;
            }

            /**
             * @brief Read the internal temperature sensor, in tenths of a degree
             *
             * @details
             * Uses the factory calibration points stored in system memory
             * (TS_CAL1 at 30 degrees, TS_CAL2 at 130 degrees, both at VDDA = 3.0 V);
             * the raw value is scaled to that VDDA using MeasureVdda() result.
             *
             * @returns Temperature, in 0.1 degrees Celsius
             */
            static int16_t ReadTemperature()
            {
                auto cal1 = *reinterpret_cast<const volatile uint16_t*>(TempSensorCal1Address);
                auto cal2 = *reinterpret_cast<const volatile uint16_t*>(TempSensorCal2Address);

                EnableTempSensorChannel();
                delay_us<120>(); // tSTART_TS
                SetSampleTime(TempSensorChannel, SampleTime::Cycles640_5);

                int32_t raw = SingleConversion(TempSensorChannel);
                raw = raw * _vdda / static_cast<int32_t>(CalibrationVdda);

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

        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _Pins, typename _DmaChannel>
        AdcCallbackType Adc<_Regs, _CommonRegs, _ClockCtrl, _IRQNumber, _Pins, _DmaChannel>::_callback = nullptr;

        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _Pins, typename _DmaChannel>
        AdcCommon::AdcError Adc<_Regs, _CommonRegs, _ClockCtrl, _IRQNumber, _Pins, _DmaChannel>::_error = AdcCommon::AdcError::NoError;

        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQNumber, typename _Pins, typename _DmaChannel>
        uint16_t Adc<_Regs, _CommonRegs, _ClockCtrl, _IRQNumber, _Pins, _DmaChannel>::_vdda = VRefNominal / 10;

        /// Pin maps list the LQFP64 inputs; extend for other packages if needed
        struct Adc1Pins
        {
            using io_pins = IO::PinList<IO::Pa0, IO::Pa1, IO::Pa2, IO::Pb14, IO::Pc0, IO::Pc1, IO::Pc2, IO::Pc3, IO::Pf0, IO::Pb12, IO::Pb1, IO::Pb0>;
            static constexpr std::array<uint8_t, 12> channels{1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 15};
            static constexpr uint8_t TempSensorChannel = 16;
            static constexpr uint8_t VBatChannel = 17;
            static constexpr uint8_t ReferenceChannel = 18;
        };

        struct Adc2Pins
        {
            using io_pins = IO::PinList<IO::Pa0, IO::Pa1, IO::Pa6, IO::Pa7, IO::Pc4, IO::Pc0, IO::Pc1, IO::Pc2, IO::Pc3, IO::Pf1, IO::Pc5, IO::Pb2, IO::Pa5, IO::Pb15, IO::Pa4>;
            static constexpr std::array<uint8_t, 15> channels{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 17};
            static constexpr uint8_t TempSensorChannel = AdcCommon::NoChannel;
            static constexpr uint8_t VBatChannel = AdcCommon::NoChannel;
            static constexpr uint8_t ReferenceChannel = AdcCommon::NoChannel;
        };

        IO_STRUCT_WRAPPER(ADC1, Adc1Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC2, Adc2Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC12_COMMON, Adc12CommonRegs, ADC_Common_TypeDef);

#if defined (ADC3)
        struct Adc3Pins
        {
            using io_pins = IO::PinList<IO::Pb1, IO::Pb13, IO::Pb0>;
            static constexpr std::array<uint8_t, 3> channels{1, 5, 12};
            static constexpr uint8_t TempSensorChannel = AdcCommon::NoChannel;
            static constexpr uint8_t VBatChannel = 17;
            static constexpr uint8_t ReferenceChannel = 18;
        };

        IO_STRUCT_WRAPPER(ADC3, Adc3Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC345_COMMON, Adc345CommonRegs, ADC_Common_TypeDef);
#endif

#if defined (ADC4)
        struct Adc4Pins
        {
            using io_pins = IO::PinList<IO::Pb12, IO::Pb14, IO::Pb15>;
            static constexpr std::array<uint8_t, 3> channels{3, 4, 5};
            static constexpr uint8_t TempSensorChannel = AdcCommon::NoChannel;
            static constexpr uint8_t VBatChannel = AdcCommon::NoChannel;
            static constexpr uint8_t ReferenceChannel = 18;
        };

        IO_STRUCT_WRAPPER(ADC4, Adc4Regs, ADC_TypeDef);
#endif

#if defined (ADC5)
        struct Adc5Pins
        {
            using io_pins = IO::PinList<IO::Pa8, IO::Pa9>;
            static constexpr std::array<uint8_t, 2> channels{1, 2};
            static constexpr uint8_t TempSensorChannel = 4;
            static constexpr uint8_t VBatChannel = 17;
            static constexpr uint8_t ReferenceChannel = 18;
        };

        IO_STRUCT_WRAPPER(ADC5, Adc5Regs, ADC_TypeDef);
#endif
    }

    template<typename _DmaChannel = void>
    using Adc1 = Private::Adc<Private::Adc1Regs, Private::Adc12CommonRegs, Clock::Adc12Clock, ADC1_2_IRQn, Private::Adc1Pins, _DmaChannel>;
    using Adc1NoDma = Adc1<>;

    template<typename _DmaChannel = void>
    using Adc2 = Private::Adc<Private::Adc2Regs, Private::Adc12CommonRegs, Clock::Adc12Clock, ADC1_2_IRQn, Private::Adc2Pins, _DmaChannel>;
    using Adc2NoDma = Adc2<>;

#if defined (ADC3)
    template<typename _DmaChannel = void>
    using Adc3 = Private::Adc<Private::Adc3Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC3_IRQn, Private::Adc3Pins, _DmaChannel>;
    using Adc3NoDma = Adc3<>;
#endif

#if defined (ADC4)
    template<typename _DmaChannel = void>
    using Adc4 = Private::Adc<Private::Adc4Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC4_IRQn, Private::Adc4Pins, _DmaChannel>;
    using Adc4NoDma = Adc4<>;
#endif

#if defined (ADC5)
    template<typename _DmaChannel = void>
    using Adc5 = Private::Adc<Private::Adc5Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC5_IRQn, Private::Adc5Pins, _DmaChannel>;
    using Adc5NoDma = Adc5<>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_G4_ADC_H
