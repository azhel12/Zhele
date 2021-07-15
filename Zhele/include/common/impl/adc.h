/**
 * @file
 * @brief Implements ADC
 * @date 2016
 * @author Konstantin Chizhov
 * @license FreeBSD
 */

#ifndef ZHELE_ADC_IMPL_COMMON_H
#define ZHELE_ADC_IMPL_COMMON_H

#if defined (ADC_CR_ADEN)
#define ADC_TYPE_1
#endif
#if defined (ADC_CR2_ADON)
#define ADC_TYPE_2
#endif

#include <delay.h>

namespace Zhele::Private
{
    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SelectClockSource(ClockSource clockSource)
    {
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::VerifyReady(unsigned readyMask)
    {
        uint32_t timeout = AdcTimeoutCycles * 4;
        while ((_Regs()->SR & readyMask) && --timeout)
        {}

        return timeout != 0;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ClockFreq()
    {
        return _ClockCtrl::ClockFreq();
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::AdcPeriodUs10(uint8_t channel)
    {
        unsigned adcTickUs4 = (4000000000u / ClockFreq());
        unsigned adcTickUs10 = adcTickUs4 * 2 + adcTickUs4 / 2;
        return (adcTickUs10 * ConvertionTimeCycles(channel) + adcTickUs10 / 2);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SampleTimeToReg(unsigned sampleTime)
    {
        if (sampleTime > 239)
            return 7;
        if (sampleTime > 71)
            return 6;
        if (sampleTime > 55)
            return 5;
        if (sampleTime > 41)
            return 4;
        if (sampleTime > 28)
            return 3;
        if (sampleTime > 13)
            return 2;
        if (sampleTime > 7)
            return 1;
        return 0;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetReference(Reference)
    {
        // Not supported.
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetDivider(AdcDivider divider)
    {
        _ClockCtrl::SetPrescaler(divider);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    uint16_t AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ReadInjected(uint8_t channel)
    {
        StartInjected(channel);
        return ReadInjected();
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetRegularCallback(AdcCallbackType callback)
    {
        _adcData.regularCallback = callback;
    }

#if defined (ADC_TYPE_1)
    
#endif
#if defined (ADC_TYPE_2)
    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ConvertionTimeCycles(uint8_t channel)
    {
        unsigned sampleTimeBits;
        if (channel <= 9)
        {
            unsigned shift = channel * 3;
            sampleTimeBits = (_Regs()->SMPR2 >> shift) & 0x07;
        }
        else
        {
            channel -= 10;
            unsigned shift = channel * 3;
            sampleTimeBits = (_Regs()->SMPR1 >> shift) & 0x07;
        }

        static const uint16_t sampleTimes[] = {1, 7, 13, 28, 41, 55, 71, 239};

        return ResolutionBits + sampleTimes[sampleTimeBits] + 1;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetSampleTime(uint8_t channel, unsigned sampleTime)
    {
        if (channel > 18)
            return;

        unsigned bitFieldValue = SampleTimeToReg(sampleTime);

        if (channel <= 9)
        {
            unsigned shift = channel * 3;
            _Regs()->SMPR2 = (_Regs()->SMPR2 & ~(0x07 << shift)) | (bitFieldValue << shift);
        }
        else
        {
            channel -= 10;
            unsigned shift = channel * 3;
            _Regs()->SMPR1 = (_Regs()->SMPR1 & ~(0x07 << shift)) | (bitFieldValue << shift);
        }
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::Init(AdcDivider divider, ClockSource clockSource, Reference reference)
    {
        _ClockCtrl::Enable();
        SelectClockSource(clockSource);
        SetDivider(divider);
        _Regs()->CR1 = 0;
        _Regs()->SQR1 = 0;
        _Regs()->SQR2 = 0;
        _Regs()->SQR3 = 0;
        _Regs()->JSQR = 0;
        
        _Regs()->CR1 = ADC_CR1_EOSIE | ADC_CR1_JEOSIE;
        _Regs()->CR2 = ADC_CR2_ADON | ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
        _Regs()->CR2 |= ADC_CR2_RSTCAL;
        while (_Regs()->CR2 & ADC_CR2_RSTCAL)
        {
        }

        delay_ms<100>(); // need delay at least one ADC cycle

        _Regs()->CR2 |= ADC_CR2_CAL;
        while (_Regs()->CR2 & ADC_CR2_CAL)
        {
        }

        _adcData.vRef = 0;
        NVIC_EnableIRQ(ADC1_IRQn);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    uint8_t AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetResolution(uint8_t bits)
    {
        (void)bits;
        return ResolutionBits;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::Disable()
    {
        _Regs()->CR1 = 0;
        _Regs()->CR2 = 0;
        _ClockCtrl::Disable();
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    template <typename InjectedTrigger, typename TriggerMode>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetInjectedTrigger(InjectedTrigger trigger, TriggerMode mode)
    {
        _Regs()->CR2 = (_Regs()->CR2 & ~(ADC_CR2_JEXTSEL | ADC_CR2_JEXTTRIG)) | ((trigger & 0x07) << ADC_CR2_JEXTSEL_Pos) | (mode << ADC_CR2_JEXTTRIG_Pos);
    }

    template <typename Pins, typename _Regs>
    static void EnableChannel(uint8_t channel)
    {
        if (channel == AdcCommon::TempSensorChannel)
        {
            _Regs()->CR2 |= ADC_CR2_TSVREFE;
        }
        else
        {
            Pins::SetConfiguration(1u << channel, Pins::Analog);
        }
    }

    static unsigned GetJsqr(const uint8_t *channels, uint8_t count, uint32_t jsqr)
    {
        jsqr = ((count - 1) << 20) | (channels[0] << 0);

        if (count > 1)
            jsqr |= (channels[1] << 5);
        if (count > 2)
            jsqr |= (channels[2] << 10);
        if (count > 3)
            jsqr |= (channels[3] << 15);
        return jsqr;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StartInjected(const uint8_t *channels, uint16_t *data, uint8_t count, AdcCallbackType callback)
    {
        if (count == 0 || count > 4)
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }

        if (!VerifyReady(ADC_SR_JSTRT))
        {
            _adcData.error = AdcError::HardwareError;
            return false;
        }

        _Regs()->CR1 |= ADC_CR1_JDISCEN;

        _adcData.injectedCallback = callback;
        _adcData.injectedData = data;

        for (unsigned i = 0; i < count; i++)
            EnableChannel<Pins, _Regs>(channels[i]);

        _Regs()->JSQR = GetJsqr(channels, count, _Regs()->JSQR);
        if (callback)
        {
            _Regs()->CR1 |= ADC_CR1_JEOCIE;
        }
        else
        {
            _Regs()->CR1 &= ~ADC_CR1_JEOCIE;
        }

        _Regs()->CR2 |= ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL;
        _Regs()->CR2 |= ADC_CR2_JSWSTART;

        return true;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ReadInjected(const uint8_t *channels, uint16_t *data, uint8_t count)
    {
        bool result = false, alreadyStarted = false;
        if (_Regs()->SR & ADC_SR_JSTRT)
        {
            unsigned jsqr = _Regs()->JSQR;
            alreadyStarted = (jsqr == GetJsqr(channels, count, jsqr));
            if (!alreadyStarted)
            {
                _adcData.error = AdcError::RegularError;
                return result;
            }
        }
        if (alreadyStarted || StartInjected(channels, data, count, 0))
        {
            unsigned status, timeout = AdcTimeoutCycles;
            do
            {
                status = _Regs()->SR;
            } while (((status & ADC_SR_JEOC) == 0) && --timeout);

            if ((status & ADC_SR_JEOC) == 0 || !timeout)
            {
                _adcData.error = AdcError::HardwareError;
            }
            else
            {
                result = true;

                data[0] = _Regs()->JDR1;
                if (count > 1)
                    data[1] = _Regs()->JDR2;
                if (count > 2)
                    data[2] = _Regs()->JDR3;
                if (count > 3)
                    data[3] = _Regs()->JDR4;
                _adcData.error = AdcError::NoError;
            }
            //Regs()->SR &= ~(ADC_SR_JEOC);
        }
        return result;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::DmaHandler(void *data, size_t size, bool success)
    {
        _DmaChannel::Disable();
        _Regs()->CR2 &= ~(ADC_CR2_DMA);
        _Regs()->SR = ~(ADC_SR_EOC);

        if (success)
        {
            if (_adcData.regularCallback)
                _adcData.regularCallback((uint16_t *)data, size);
        }
        else
            _adcData.error = AdcError::TransferError;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::IrqHandler()
    {
        unsigned sr = _Regs()->SR;

        if (sr & ADC_SR_JEOC)
        {
            unsigned count = ((_Regs()->JSQR & ADC_JSQR_JL) >> 20) + 1;
            uint16_t *data = _adcData.injectedData;
            if (data)
            {
                data[0] = _Regs()->JDR1;
                if (count > 1)
                    data[1] = _Regs()->JDR2;
                if (count > 2)
                    data[2] = _Regs()->JDR3;
                if (count > 3)
                    data[3] = _Regs()->JDR4;
                _adcData.error = AdcError::NoError;

                if (_adcData.injectedCallback)
                    _adcData.injectedCallback(data, count);
            }
        }
        // reset all flags
        _Regs()->SR &= ~(ADC_SR_JEOC | ADC_SR_JSTRT);
        NVIC_ClearPendingIRQ(ADC1_IRQn);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StartInjected(uint8_t channel)
    {
        if (channel > ChannelCount)
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }

        if (!VerifyReady(ADC_SR_JSTRT))
        {
            _adcData.error = AdcError::HardwareError;
            return false;
        }

        _Regs()->CR1 |= ADC_CR1_DISCEN;
        _Regs()->SR = ~(ADC_SR_JEOC);
        _Regs()->JSQR = (unsigned)channel;

        EnableChannel<Pins, _Regs>(channel);

        _Regs()->CR2 |= ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL;
        _Regs()->CR2 |= ADC_CR2_JSWSTART;
        return true;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    uint16_t AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ReadInjected()
    {
        unsigned status, timeout = AdcTimeoutCycles;
        uint16_t result = 0xffff;
        do
        {
            status = _Regs()->SR;
        } while (((status & (ADC_SR_JEOC)) == 0) && --timeout);

        if ((status & (ADC_SR_JEOC)) == 0)
        {
            _adcData.error = AdcError::HardwareError;
        }
        else
        {
            result = _Regs()->JDR1;
            _adcData.error = AdcError::NoError;
        }

        _Regs()->SR &= ~(ADC_SR_JEOC);
        return result;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::InjectedReady()
    {
        return (_Regs()->SR & (ADC_SR_JEOC)) == (ADC_SR_JEOC);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StopInjected()
    {
        _Regs()->SR &= ~(ADC_SR_JSTRT | ADC_SR_JEOC);
        _Regs()->JSQR = 0;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    template<typename RegularTrigger, typename TriggerMode>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::SetRegularTrigger(RegularTrigger trigger, TriggerMode mode)
    {
        _Regs()->CR2 = (_Regs()->CR2 & ~(ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG)) | ((trigger & 0x0f) << ADC_CR2_EXTSEL) | (mode << ADC_CR2_EXTTRIG_Pos);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StartRegular(std::initializer_list<uint8_t> channels, uint16_t *dataBuffer, uint16_t scanCount)
    {
        if (scanCount == 0 || channels.size() == 0)
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }

        if (!VerifyReady(ADC_SR_STRT))
        {
            _adcData.error = AdcError::NotReady;
            return false;
        }

        _Regs()->SR &= ~(ADC_SR_STRT | ADC_SR_EOC);

        if (channels.size() <= MaxRegular)
        {
            _Regs()->SQR1 = ((channels.size() - 1) << 20);
            _Regs()->SQR3 = 0;
            _Regs()->SQR2 = 0;
            _Regs()->SQR4 = 0;
            unsigned i = 0;
            for (uint8_t channel : channels)
            {
                Pins::SetConfiguration(1u << channel, Pins::Analog);
                if (i < 6)
                {
                    _Regs()->SQR3 |= (channel & 0x1f) << 5 * (i);
                }
                else if (i < 12)
                {
                    _Regs()->SQR2 |= (channel & 0x1f) << 5 * (i - 6);
                }
                else
                {
                    _Regs()->SQR1 |= (channel & 0x1f) << 5 * (i - 12);
                }
                i++;
            }

            _DmaChannel::SetTransferCallback(DmaHandler);
            //DmaChannel::SetRequest(channelNum);
            _DmaChannel::Transfer(DmaBase::Periph2Mem | DmaBase::MemIncrement | DmaBase::PriorityHigh | DmaBase::PSize16Bits | DmaBase::MSize16Bits,
                                dataBuffer, &_Regs()->DR, channels.size() * scanCount);

            _adcData.error = AdcError::NoError;

            uint32_t controlReg = _Regs()->CR2;
            controlReg |= ADC_CR2_DMA;
            if (scanCount > 1)
                controlReg |= ADC_CR2_CONT;
            _Regs()->CR2 = controlReg;

            // start conversion now if no external trigger selected
            if ((controlReg & ADC_CR2_EXTTRIG) == 0)
                _Regs()->CR2 |= ADC_CR2_SWSTART;

            return true;
        }
        else
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StartRegular(const uint8_t *channels, uint8_t channelsCount, uint16_t *dataBuffer, uint16_t scanCount)
    {
        if (scanCount == 0 || channelsCount == 0)
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }

        if (!VerifyReady(ADC_SR_STRT))
        {
            _adcData.error = AdcError::NotReady;
            return false;
        }

        _Regs()->SR &= ADC_SR_STRT | ADC_SR_EOC;

        if (channelsCount <= MaxRegular)
        {
            _Regs()->SQR1 = ((channelsCount - 1) << 20);
            _Regs()->SQR3 = 0;
            _Regs()->SQR2 = 0;
            _Regs()->SQR4 = 0;
            for (unsigned i = 0; i < channelsCount; i++)
            {
                Pins::SetConfiguration(1u << channels[i], Pins::Analog);
                if (i < 6)
                {
                    _Regs()->SQR3 |= (channels[i] & 0x1f) << 5 * (i);
                }
                else if (i < 12)
                {
                    _Regs()->SQR2 |= (channels[i] & 0x1f) << 5 * (i - 6);
                }
                else
                {
                    _Regs()->SQR1 |= (channels[i] & 0x1f) << 5 * (i - 12);
                }
            }

            _DmaChannel::SetTransferCallback(DmaHandler);
            _DmaChannel::Transfer(DmaBase::Periph2Mem | DmaBase::MemIncrement | DmaBase::PriorityHigh | DmaBase::PSize16Bits | DmaBase::MSize16Bits,
                                dataBuffer, &_Regs()->DR, channelsCount * scanCount);

            _adcData.error = AdcError::NoError;

            uint32_t controlReg = _Regs()->CR2;
            controlReg |= ADC_CR2_DMA;
            if (scanCount > 1)
                controlReg |= ADC_CR2_CONT;
            _Regs()->CR2 = controlReg;

            // start conversion now if no external trigger selected
            if ((controlReg & ADC_CR2_EXTTRIG) == 0)
                _Regs()->CR2 |= ADC_CR2_SWSTART;

            return true;
        }
        else
        {
            _adcData.error = AdcError::ArgumentError;
            return false;
        }
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    template <typename... _Pins>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StartRegular(uint16_t *dataBuffer, uint16_t scanCount)
    {
        return StartRegular({Pins::template PinIndex<_Pins>::Value...}, dataBuffer, scanCount);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    bool AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::RegularReady()
    {
        return _DmaChannel::Ready();
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    void AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::StopRegular()
    {
        _DmaChannel::Disable();
        _Regs()->SR &= ~(ADC_SR_STRT | ADC_SR_EOC);
        _Regs()->SQR1 = 0;
        _Regs()->SQR2 = 0;
        _Regs()->SQR3 = 0;
    }
#endif

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    template <typename _Pin>
    uint16_t AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ReadInjected()
    {
        const int index = Pins::template PinIndex<_Pin>::value;
        static_assert(index >= 0);
        return ReadInjected(index);
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    template <typename _Pin>
    constexpr unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ChannelNum()
    {
        return Pins::template PinIndex<_Pin>::Value;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::AdcError AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::GetError()
    {
        return _adcData.error;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    int16_t AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ReadTemperature()
    {
        SetSampleTime(TempSensorChannel, 250);
        uint16_t rawValue = ReadInjected(TempSensorChannel);
        int16_t v25 = 14100;	// x10000
        int16_t Avg_Slope = 43; // x10000
        int16_t value = (v25 - ToVolts(rawValue)) / Avg_Slope + 25;
        return value;
    }

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    AdcData AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::_adcData;

    template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
    unsigned AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>::ToVolts(uint16_t value)
    {
        if(_adcData.vRef == 0)
        {
            for(int i = 0; i < 4; i++)
            {
                _adcData.vRef += ReadInjected(ReferenceChannel);
            }
            _adcData.vRef /= 4;
        }
        return VRefNominal * value / _adcData.vRef;
    }
}

#endif //! ZHELE_ADC_IMPL_COMMON_H