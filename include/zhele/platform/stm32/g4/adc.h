/**
 * @file
 * @brief Implements ADC for stm32g4 series
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 *
 * @details
 * The G4 ADC IP (SAR ADC with ADEN/ADCAL/ISR/ CFGR/SQRx/SMPRx registers) is not compatible
 * with the register layout the common ADC engine (../common/adc.h) was written for
 * (F1-style SR/CR1/CR2/SQRx registers), so this is a self-contained driver instead of an
 * instantiation of Private::AdcBase.
 *
 * Not implemented in this version: injected channels, external triggers, oversampling,
 * differential mode, dual (simultaneous) mode. Internal channels (temperature sensor,
 * VBAT, VREFINT) are only wired to ADC1 (and ADC5 on some devices) — see RM0440.
 */

#ifndef ZHELE_PLATFORM_STM32_G4_ADC_H
#define ZHELE_PLATFORM_STM32_G4_ADC_H

#include <stm32g4xx.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include <zhele/delay.h>
#include <zhele/pinlist.h>

#include "../common/adc.h"

#include <array>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        /// Factory calibration values (RM0440 §21.4.34, DS12589/DS12288)
        static constexpr uint32_t Adc_VrefintCalAddr = 0x1FFF75AAUL;
        static constexpr uint32_t Adc_TempSensorCal1Addr = 0x1FFF75A8UL;
        static constexpr uint32_t Adc_TempSensorCal2Addr = 0x1FFF75CAUL;
        static constexpr int32_t Adc_TempSensorCal1Temp = 30;
        static constexpr int32_t Adc_TempSensorCal2Temp = 110;
        static constexpr uint16_t Adc_VrefintCalVref = 3000; // mV, Vdda at which VREFINT/TS were calibrated

        /**
         * @brief Implements STM32G4 ADC (SAR ADC, ADEN/ADCAL-based)
         *
         * @tparam _Regs ADC instance register wrapper
         * @tparam _CommonRegs ADC12_COMMON/ADC345_COMMON register wrapper (shared by a pair of ADCs)
         * @tparam _ClockCtrl Clock control (Clock::Adc12Clock or Clock::Adc345Clock)
         * @tparam _IRQn ADC IRQ number
         * @tparam _PinMap Pin map: exposes `io_pins` (PinList of bonded input pins) and a
         *         `channels` array giving the ADC channel number for each pin (by index)
         * @tparam _DmaChannel DMA channel used for StartRegular()/StopRegular() (optional)
         */
        template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQn, typename _PinMap, typename _DmaChannel = void>
        class AdcG4 : public AdcCommon
        {
        public:
            static const uint8_t ResolutionBits = 12;

            // Internal channels (ADC1 only, see RM0440 §21.4.19)
            static const uint8_t TempSensorChannel = 16;
            static const uint8_t VBatChannel = 17;
            static const uint8_t ReferenceChannel = 18;

            /**
             * @brief Initializes ADC: enables clock, exits deep-power-down, enables the
             * voltage regulator, calibrates (single-ended), configures input pins as
             * analog and enables the ADC (ADEN)
             *
             * @par Returns
             *  Nothing
             */
            static void Init();

            /**
             * @brief Sets sample time for given channel
             *
             * @param channel Channel number (0..18)
             * @param sampleTime Sample time selector (0..7, see ADC_SMPR1_SMPx)
             *
             * @par Returns
             *  Nothing
             */
            static void SetSampleTime(uint8_t channel, uint8_t sampleTime);

            /**
             * @brief Sets sample time for channel connected to given pin
             */
            template<typename Pin>
            static void SetSampleTime(uint8_t sampleTime);

            /**
             * @brief Returns ADC channel number for given pin (compile time)
             */
            template<typename Pin>
            static constexpr uint8_t ChannelNum();

            /**
             * @brief Performs single blocking conversion on given channel
             *
             * @param channel Channel number
             *
             * @returns Raw conversion result (12 bit)
             */
            static uint16_t ReadSingle(uint8_t channel);

            /**
             * @brief Performs single blocking conversion on channel connected to given pin
             */
            template<typename Pin>
            static uint16_t ReadSingle();

            /**
             * @brief Starts continuous regular sequence conversion with DMA transfer
             *
             * @param channels Channels array
             * @param channelsCount Channels count (1..16)
             * @param dataBuffer Destination buffer (size must be at least channelsCount * scanCount)
             * @param scanCount Sequence repeats stored in buffer before DMA circles back
             *
             * @retval true Started
             * @retval false Invalid arguments
             */
            static bool StartRegular(const uint8_t* channels, uint8_t channelsCount, uint16_t* dataBuffer, uint16_t scanCount);

            /**
             * @brief Stops regular sequence conversion started by StartRegular()
             *
             * @par Returns
             *  Nothing
             */
            static void StopRegular();

            /**
             * @brief Enables VREFINT path (common to the ADC pair)
             */
            static void EnableVref();
            static void DisableVref();

            /**
             * @brief Enables temperature sensor path (common to the ADC pair, ADC1 only)
             */
            static void EnableTemperatureSensor();
            static void DisableTemperatureSensor();

            /**
             * @brief Enables VBAT/3 path (common to the ADC pair, ADC1 only)
             */
            static void EnableVBat();
            static void DisableVBat();

            /**
             * @brief Measures actual Vdda (mV) via VREFINT and factory calibration value,
             * caches it for ToVolts()/ReadTemperature()
             *
             * @returns Measured Vdda in millivolts
             */
            static uint16_t MeasureVdda();

            /**
             * @brief Converts raw conversion result to millivolts (uses nominal 3300 mV
             * Vdda unless MeasureVdda() was called before)
             */
            static unsigned ToVolts(uint16_t value);

            /**
             * @brief Reads internal temperature sensor (ADC1 only)
             *
             * @returns Temperature, degrees Celsius
             */
            static int16_t ReadTemperature();

            /**
             * @brief Clears pending interrupt flags (EOC/EOS/OVR/...)
             *
             * @par Returns
             *  Nothing
             */
            static void IrqHandler();

        private:
            static void Calibrate();
            static void WriteSequenceSlot(uint8_t index, uint8_t channel);

            static inline uint16_t _vddaMv = 3300;
        };
    }

    struct Adc1Pins
    {
        using io_pins = IO::PinList<IO::Pa0, IO::Pa1, IO::Pa2, IO::Pb14, IO::Pc0, IO::Pc1, IO::Pc2, IO::Pc3, IO::Pf0, IO::Pb12, IO::Pb1, IO::Pb0>;
        static constexpr std::array<uint8_t, 12> channels{1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 15};
    };

    struct Adc2Pins
    {
        using io_pins = IO::PinList<IO::Pa0, IO::Pa1, IO::Pa6, IO::Pa7, IO::Pc4, IO::Pc0, IO::Pc1, IO::Pc2, IO::Pc3, IO::Pf1, IO::Pc5, IO::Pb2, IO::Pa5, IO::Pb15, IO::Pa4>;
        static constexpr std::array<uint8_t, 15> channels{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 17};
    };

    namespace Private
    {
        IO_STRUCT_WRAPPER(ADC1, Adc1Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC2, Adc2Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC12_COMMON, Adc12CommonRegs, ADC_Common_TypeDef);
    }

    template<typename _DmaChannel = void>
    using Adc1 = Private::AdcG4<Private::Adc1Regs, Private::Adc12CommonRegs, Clock::Adc12Clock, ADC1_2_IRQn, Adc1Pins, _DmaChannel>;
    using Adc1NoDma = Adc1<>;

    template<typename _DmaChannel = void>
    using Adc2 = Private::AdcG4<Private::Adc2Regs, Private::Adc12CommonRegs, Clock::Adc12Clock, ADC1_2_IRQn, Adc2Pins, _DmaChannel>;
    using Adc2NoDma = Adc2<>;

#if defined (ADC3)
    // Category 3/4 devices (G47x/G48x/G49x). Pin maps below list only the channels bonded
    // on the LQFP64 package; extend for other packages/channels if needed.
    struct Adc3Pins
    {
        using io_pins = IO::PinList<IO::Pb1, IO::Pb13, IO::Pb0>;
        static constexpr std::array<uint8_t, 3> channels{1, 5, 12};
    };
    struct Adc4Pins
    {
        using io_pins = IO::PinList<IO::Pb12, IO::Pb14, IO::Pb15>;
        static constexpr std::array<uint8_t, 3> channels{3, 4, 5};
    };
    struct Adc5Pins
    {
        using io_pins = IO::PinList<IO::Pa8, IO::Pa9>;
        static constexpr std::array<uint8_t, 2> channels{1, 2};
    };

    namespace Private
    {
        IO_STRUCT_WRAPPER(ADC3, Adc3Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC4, Adc4Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC5, Adc5Regs, ADC_TypeDef);
        IO_STRUCT_WRAPPER(ADC345_COMMON, Adc345CommonRegs, ADC_Common_TypeDef);
    }

    template<typename _DmaChannel = void>
    using Adc3 = Private::AdcG4<Private::Adc3Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC3_IRQn, Adc3Pins, _DmaChannel>;
    using Adc3NoDma = Adc3<>;

    template<typename _DmaChannel = void>
    using Adc4 = Private::AdcG4<Private::Adc4Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC4_IRQn, Adc4Pins, _DmaChannel>;
    using Adc4NoDma = Adc4<>;

    template<typename _DmaChannel = void>
    using Adc5 = Private::AdcG4<Private::Adc5Regs, Private::Adc345CommonRegs, Clock::Adc345Clock, ADC5_IRQn, Adc5Pins, _DmaChannel>;
    using Adc5NoDma = Adc5<>;
#endif
}

namespace Zhele::Private
{
    #define ADCG4_TEMPLATE_ARGS template<typename _Regs, typename _CommonRegs, typename _ClockCtrl, IRQn_Type _IRQn, typename _PinMap, typename _DmaChannel>
    #define ADCG4_TEMPLATE_QUALIFIER AdcG4<_Regs, _CommonRegs, _ClockCtrl, _IRQn, _PinMap, _DmaChannel>

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::Calibrate()
    {
        _Regs()->CR &= ~ADC_CR_ADCALDIF;
        _Regs()->CR |= ADC_CR_ADCAL;
        while ((_Regs()->CR & ADC_CR_ADCAL) != 0) continue;
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::Init()
    {
        _ClockCtrl::Enable();

        // Synchronous clock, HCLK/4 (safe up to 170 MHz AHB, ADC clock <= 60 MHz)
        _CommonRegs()->CCR = (_CommonRegs()->CCR & ~ADC_CCR_CKMODE) | ADC_CCR_CKMODE_0 | ADC_CCR_CKMODE_1;

        _Regs()->CR &= ~ADC_CR_DEEPPWD;
        _Regs()->CR |= ADC_CR_ADVREGEN;
        delay_us<20>(); // tADCVREG_STUP

        Calibrate();

        _PinMap::io_pins::Enable();
        _PinMap::io_pins::SetConfiguration(_PinMap::io_pins::Configuration::Analog);

        _Regs()->ISR = ADC_ISR_ADRDY;
        _Regs()->CR |= ADC_CR_ADEN;
        while ((_Regs()->ISR & ADC_ISR_ADRDY) == 0) continue;
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::SetSampleTime(uint8_t channel, uint8_t sampleTime)
    {
        uint32_t value = static_cast<uint32_t>(sampleTime & 0x7);
        if (channel <= 9)
        {
            uint32_t shift = channel * 3u;
            _Regs()->SMPR1 = (_Regs()->SMPR1 & ~(0x7u << shift)) | (value << shift);
        }
        else
        {
            uint32_t shift = (channel - 10u) * 3u;
            _Regs()->SMPR2 = (_Regs()->SMPR2 & ~(0x7u << shift)) | (value << shift);
        }
    }

    ADCG4_TEMPLATE_ARGS
    template<typename Pin>
    void ADCG4_TEMPLATE_QUALIFIER::SetSampleTime(uint8_t sampleTime)
    {
        SetSampleTime(ChannelNum<Pin>(), sampleTime);
    }

    ADCG4_TEMPLATE_ARGS
    template<typename Pin>
    constexpr uint8_t ADCG4_TEMPLATE_QUALIFIER::ChannelNum()
    {
        constexpr int index = _PinMap::io_pins::template IndexOf<Pin>;
        static_assert(index >= 0, "Pin is not a member of this ADC's pin map");
        return _PinMap::channels[index];
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::WriteSequenceSlot(uint8_t index, uint8_t channel)
    {
        uint32_t value = static_cast<uint32_t>(channel & 0x1F);
        if (index < 4)
            _Regs()->SQR1 |= value << (6 + index * 6);
        else if (index < 9)
            _Regs()->SQR2 |= value << ((index - 4) * 6);
        else if (index < 14)
            _Regs()->SQR3 |= value << ((index - 9) * 6);
        else
            _Regs()->SQR4 |= value << ((index - 14) * 6);
    }

    ADCG4_TEMPLATE_ARGS
    uint16_t ADCG4_TEMPLATE_QUALIFIER::ReadSingle(uint8_t channel)
    {
        _Regs()->SQR1 = static_cast<uint32_t>(channel & 0x1F) << ADC_SQR1_SQ1_Pos;
        _Regs()->ISR = ADC_ISR_EOC | ADC_ISR_EOS;
        _Regs()->CR |= ADC_CR_ADSTART;
        while ((_Regs()->ISR & ADC_ISR_EOC) == 0) continue;
        return static_cast<uint16_t>(_Regs()->DR);
    }

    ADCG4_TEMPLATE_ARGS
    template<typename Pin>
    uint16_t ADCG4_TEMPLATE_QUALIFIER::ReadSingle()
    {
        return ReadSingle(ChannelNum<Pin>());
    }

    ADCG4_TEMPLATE_ARGS
    bool ADCG4_TEMPLATE_QUALIFIER::StartRegular(const uint8_t* channels, uint8_t channelsCount, uint16_t* dataBuffer, uint16_t scanCount)
    {
        static_assert(!std::is_void_v<_DmaChannel>, "StartRegular requires a DMA channel");

        if (channelsCount == 0 || channelsCount > MaxRegular || dataBuffer == nullptr || scanCount == 0)
            return false;

        _Regs()->SQR1 = 0;
        _Regs()->SQR2 = 0;
        _Regs()->SQR3 = 0;
        _Regs()->SQR4 = 0;
        for (uint8_t i = 0; i < channelsCount; ++i)
            WriteSequenceSlot(i, channels[i]);
        _Regs()->SQR1 = (_Regs()->SQR1 & ~ADC_SQR1_L) | (static_cast<uint32_t>(channelsCount - 1) << ADC_SQR1_L_Pos);

        _Regs()->CFGR = (_Regs()->CFGR & ~(ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG))
            | ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

        _DmaChannel::Transfer(
            _DmaChannel::Periph2Mem | _DmaChannel::MemIncrement | _DmaChannel::Circular
                | _DmaChannel::PSize16Bits | _DmaChannel::MSize16Bits,
            dataBuffer, &_Regs()->DR, static_cast<uint32_t>(channelsCount) * scanCount);

        _Regs()->CR |= ADC_CR_ADSTART;

        return true;
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::StopRegular()
    {
        static_assert(!std::is_void_v<_DmaChannel>, "StopRegular requires a DMA channel");

        if ((_Regs()->CR & ADC_CR_ADSTART) != 0)
        {
            _Regs()->CR |= ADC_CR_ADSTP;
            while ((_Regs()->CR & ADC_CR_ADSTP) != 0) continue;
        }
        _Regs()->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
        _DmaChannel::Disable();
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::EnableVref() { _CommonRegs()->CCR |= ADC_CCR_VREFEN; }
    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::DisableVref() { _CommonRegs()->CCR &= ~ADC_CCR_VREFEN; }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::EnableTemperatureSensor() { _CommonRegs()->CCR |= ADC_CCR_VSENSESEL; }
    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::DisableTemperatureSensor() { _CommonRegs()->CCR &= ~ADC_CCR_VSENSESEL; }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::EnableVBat() { _CommonRegs()->CCR |= ADC_CCR_VBATSEL; }
    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::DisableVBat() { _CommonRegs()->CCR &= ~ADC_CCR_VBATSEL; }

    ADCG4_TEMPLATE_ARGS
    uint16_t ADCG4_TEMPLATE_QUALIFIER::MeasureVdda()
    {
        EnableVref();
        delay_us<12>(); // tSTART, VREFINT
        uint16_t raw = ReadSingle(ReferenceChannel);
        uint16_t cal = *reinterpret_cast<const volatile uint16_t*>(Adc_VrefintCalAddr);
        _vddaMv = static_cast<uint16_t>((static_cast<uint32_t>(Adc_VrefintCalVref) * cal) / raw);
        return _vddaMv;
    }

    ADCG4_TEMPLATE_ARGS
    unsigned ADCG4_TEMPLATE_QUALIFIER::ToVolts(uint16_t value)
    {
        return static_cast<unsigned>(value) * _vddaMv / ((1u << ResolutionBits) - 1);
    }

    ADCG4_TEMPLATE_ARGS
    int16_t ADCG4_TEMPLATE_QUALIFIER::ReadTemperature()
    {
        EnableTemperatureSensor();
        delay_us<10>(); // tSTART, temperature sensor
        uint16_t raw = ReadSingle(TempSensorChannel);
        uint16_t cal1 = *reinterpret_cast<const volatile uint16_t*>(Adc_TempSensorCal1Addr);
        uint16_t cal2 = *reinterpret_cast<const volatile uint16_t*>(Adc_TempSensorCal2Addr);

        // Correct raw value for actual Vdda vs the 3.0 V factory calibration condition
        int32_t correctedRaw = static_cast<int32_t>(raw) * static_cast<int32_t>(_vddaMv) / Adc_VrefintCalVref;

        return static_cast<int16_t>(
            ((correctedRaw - static_cast<int32_t>(cal1)) * (Adc_TempSensorCal2Temp - Adc_TempSensorCal1Temp))
                / (static_cast<int32_t>(cal2) - static_cast<int32_t>(cal1))
            + Adc_TempSensorCal1Temp);
    }

    ADCG4_TEMPLATE_ARGS
    void ADCG4_TEMPLATE_QUALIFIER::IrqHandler()
    {
        _Regs()->ISR = _Regs()->ISR;
    }

    #undef ADCG4_TEMPLATE_ARGS
    #undef ADCG4_TEMPLATE_QUALIFIER
}

#endif //! ZHELE_PLATFORM_STM32_G4_ADC_H
