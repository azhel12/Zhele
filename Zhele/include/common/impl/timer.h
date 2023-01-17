/**
 * @file
 * @brief Macros, templates for timers
 * @author Konstantin Chizhov
 * @date 2016
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_IMPL_COMMON_H
#define ZHELE_TIMER_IMPL_COMMON_H

namespace Zhele::Timers::Private
{

    #define BASETIMER_TEMPLATE_ARGS template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber>
    #define BASETIMER_TEMPLATE_QUALIFIER BaseTimer<_Regs, _ClockEnReg, _IRQNumber>

    BASETIMER_TEMPLATE_ARGS
    unsigned BASETIMER_TEMPLATE_QUALIFIER::GetClockFreq()
    {
        return _ClockEnReg::ClockFreq();
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Enable()
    {
        _ClockEnReg::Enable();
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Disable()
    {
        _ClockEnReg::Disable();
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetCounterMode(CounterMode counterMode)
    {
        _Regs()->CR1 = (_Regs()->CR1 & ~TIM_CR1_CMS_Msk) | static_cast<uint16_t>(counterMode);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetCounterValue(BASETIMER_TEMPLATE_QUALIFIER::Counter counter)
    {
        _Regs()->CNT = counter;
    }

    BASETIMER_TEMPLATE_ARGS
    typename BASETIMER_TEMPLATE_QUALIFIER::Counter BASETIMER_TEMPLATE_QUALIFIER::GetCounterValue()
    {
        return _Regs()->CNT;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::ResetCounterValue()
    {
        _Regs()->CNT = 0;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetPrescaler(BASETIMER_TEMPLATE_QUALIFIER::Prescaler prescaler)
    {
        _Regs()->PSC = prescaler;
    }

    BASETIMER_TEMPLATE_ARGS
    typename BASETIMER_TEMPLATE_QUALIFIER::Counter BASETIMER_TEMPLATE_QUALIFIER::GetPrescaler()
    {
        return _Regs()->PSC;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetPeriod(BASETIMER_TEMPLATE_QUALIFIER::Counter period)
    {
        _Regs()->ARR = period;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetPeriodAndUpdate(BASETIMER_TEMPLATE_QUALIFIER::Counter value)
    {
        _Regs()->ARR = value;
        _Regs()->EGR |= TIM_EGR_UG;
    }

    BASETIMER_TEMPLATE_ARGS
    typename BASETIMER_TEMPLATE_QUALIFIER::Counter BASETIMER_TEMPLATE_QUALIFIER::GetPeriod()
    {
        return _Regs()->ARR;
    }


    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::EnableOnePulseMode()
    {
        _Regs()->CR1 |= TIM_CR1_OPM;
    }    

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DisableOnePulseMode()
    {
        _Regs()->CR1 &= ~TIM_CR1_OPM;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Stop()
    {
        _Regs()->CR1 = 0;
        _Regs()->CR2 = 0;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Start()
    {
        _Regs()->EGR = TIM_EGR_UG;
        _Regs()->CR1 |= (TIM_CR1_CEN | TIM_CR1_URS);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::EnableInterrupt(Interrupt interruptMask)
    {
        _Regs()->DIER |= static_cast<uint16_t>(interruptMask);
        NVIC_EnableIRQ(_IRQNumber);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DisableInterrupt(Interrupt interruptMask)
    {
        _Regs()->DIER &= ~(static_cast<uint16_t>(interruptMask));
    }

    BASETIMER_TEMPLATE_ARGS
    bool BASETIMER_TEMPLATE_QUALIFIER::IsInterrupt()
    {
        return (_Regs()->SR & TIM_SR_UIF) != 0;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::ClearInterruptFlag()
    {
        _Regs()->SR = 0;
        NVIC_ClearPendingIRQ(_IRQNumber);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetMasterMode(BASETIMER_TEMPLATE_QUALIFIER::MasterMode mode)
    {
        _Regs()->CR2 |= static_cast<uint32_t>(mode);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DmaRequestEnable()
    {
        _Regs()->DIER |= TIM_DIER_UDE;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DmaRequestDisable()
    {
        _Regs()->DIER &= ~TIM_DIER_UDE;
    }

    #define GPTIMER_TEMPLATE_ARGS template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
    #define GPTIMER_TEMPLATE_QUALIFIER GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>

    GPTIMER_TEMPLATE_ARGS
    void GPTIMER_TEMPLATE_QUALIFIER::SlaveMode::EnableSlaveMode(Mode mode)
    {
        _Regs()->SMCR = (_Regs()->SMCR & ~TIM_SMCR_SMS_Msk) | static_cast<uint16_t>(mode);
    }

    GPTIMER_TEMPLATE_ARGS
    void GPTIMER_TEMPLATE_QUALIFIER::SlaveMode::DisableSlaveMode()
    {
        _Regs()->SMCR &= ~TIM_SMCR_SMS_Msk;
    }

    GPTIMER_TEMPLATE_ARGS
    void GPTIMER_TEMPLATE_QUALIFIER::SlaveMode::SelectTrigger(Trigger trigger)
    {
        _Regs()->SMCR = (_Regs()->SMCR & ~TIM_SMCR_TS_Msk) | static_cast<uint16_t>(trigger);
    }

    GPTIMER_TEMPLATE_ARGS
    void GPTIMER_TEMPLATE_QUALIFIER::SlaveMode::SetTriggerPrescaler(ExternalTriggerPrescaler prescaler)
    {
        _Regs()->SMCR = (_Regs()->SMCR & ~TIM_SMCR_ETPS_Msk) | static_cast<uint16_t>(prescaler);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::EnableInterrupt()
    {
        _Regs()->DIER |= (TIM_DIER_CC1IE << _ChannelNumber);
        NVIC_EnableIRQ(_IRQNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::DisableInterrupt()
    {
        _Regs()->DIER &= ~(TIM_DIER_CC1IE << _ChannelNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    bool GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::IsInterrupt()
    {
        return _Regs()->SR & (TIM_SR_CC1IF << _ChannelNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::ClearInterruptFlag()
    {
        _Regs()->SR = ~(TIM_SR_CC1IF << _ChannelNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::EnableDmaRequest()
    {
        _Regs()->DIER |= (TIM_DIER_CC1DE << _ChannelNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::DisableDmaRequest()
    {
        _Regs()->DIER &= ~(TIM_DIER_CC1DE << _ChannelNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::Enable()
    {
        _Regs()->CCER |= (TIM_CCER_CC1E << (_ChannelNumber * 4));
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::ChannelBase<_ChannelNumber>::Disable()
    {
        _Regs()->CCER &= ~(TIM_CCER_CC1E << (_ChannelNumber * 4));
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::InputCapture<_ChannelNumber>::SetCapturePolarity(CapturePolarity polarity)
    {
        _Regs()->CCER = (_Regs()->CCER & ~((TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP) << (_ChannelNumber * 4))) | (polarity << (_ChannelNumber * 4));
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::InputCapture<_ChannelNumber>::SetCaptureMode(CaptureMode mode)
    {
        Channel::ModeBitField::Set(mode);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    typename GPTIMER_TEMPLATE_QUALIFIER::Base::Counter GPTIMER_TEMPLATE_QUALIFIER::InputCapture<_ChannelNumber>::GetValue()
    {
        return (&_Regs()->CCR1)[_ChannelNumber];
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::OutputCompare<_ChannelNumber>::SetPulse(typename Base::Counter pulse)
    {
        (&_Regs()->CCR1)[_ChannelNumber] = pulse;
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    typename GPTIMER_TEMPLATE_QUALIFIER::Base::Counter GPTIMER_TEMPLATE_QUALIFIER::OutputCompare<_ChannelNumber>::GetPulse()
    {
        return (&_Regs()->CCR1)[_ChannelNumber];
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::OutputCompare<_ChannelNumber>::SetOutputPolarity(OutputPolarity polarity)
    {
        _Regs()->CCER = (_Regs()->CCER & ~((TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP) << (_ChannelNumber * 4))) | (polarity << (_ChannelNumber * 4));
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::OutputCompare<_ChannelNumber>::SetOutputMode(OutputMode mode)
    {
        _Regs()->CCER = (_Regs()->CCER & ~(TIM_CCER_CC1NP << (_ChannelNumber * 4))) | (TIM_CCER_CC1E << (_ChannelNumber * 4));
        Channel::ModeBitField::Set(mode);
        _Regs()->BDTR |= TIM_BDTR_MOE;
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::PWMGeneration<_ChannelNumber>::SetOutputFastMode(FastMode mode)
    {
        _Regs()->CCER = (_Regs()->CCER & ~TIM_CCMR1_OC1FE) | static_cast<uint16_t>(mode);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::PWMGeneration<_ChannelNumber>::SelectPins(int pinNumber)
    {
        using Type = typename Pins::DataType;
        Type mask = 1 << pinNumber;
        Pins::Enable();
        Pins::SetSpeed(mask, Pins::Speed::Slow);
        Pins::SetDriverType(mask, Pins::DriverType::PushPull);
        Base::SelectPins(pinNumber);
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    template<unsigned PinNumber>
    void GPTIMER_TEMPLATE_QUALIFIER::PWMGeneration<_ChannelNumber>::SelectPins()
    {
        using Pin = typename Pins::template Pin<PinNumber>;
        Pin::Port::Enable();
        Pin::template SetSpeed<Pin::Speed::Slow>();
        Pin::template SetDriverType<Pins::DriverType::PushPull>();
        Base::template SelectPins<Pin>();
    }

    GPTIMER_TEMPLATE_ARGS
    template<unsigned _ChannelNumber>
    template<typename Pin>
    void GPTIMER_TEMPLATE_QUALIFIER::PWMGeneration<_ChannelNumber>::SelectPins()
    {
        const int index = Pins::template IndexOf<Pin>;
        static_assert(index >= 0);
        Pin::Port::Enable();
        Pin::template SetSpeed<Pin::Speed::Slow>();
        Pin::template SetDriverType<Pins::DriverType::PushPull>();
        Base::template SelectPins<Pin>();
    }

    #define ADVANCED_TIMER_TEMPLATE_ARGS template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
    #define ADVANCED_TIMER_TEMPLATE_QUALIFIER AdvancedTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>

    ADVANCED_TIMER_TEMPLATE_ARGS
    void ADVANCED_TIMER_TEMPLATE_QUALIFIER::SetRepetitionCounter(uint8_t repetitionCounter)
    {
        _Regs()->RCR = repetitionCounter;
    }

    ADVANCED_TIMER_TEMPLATE_ARGS
    uint8_t ADVANCED_TIMER_TEMPLATE_QUALIFIER::GetRepetitionCounter()
    {
        return _Regs()->RCR & 0xff;
    }
}
#endif //! ZHELE_TIMER_IMPL_COMMON_H