/**
 * @file
 * Out-of-line definitions for the CH32 timer templates declared in
 * common/timer.h (see that file for the register/bit-name notes).
 *
 * The nested GPTimer::OutputCompare<_Channel> members stay inline in the
 * declaration header — out-of-lining a member of a nested template of a
 * template buys nothing but noise, so only the top-level BaseTimer and
 * AdvancedTimer bodies live here.
 *
 * @author Aleksei Zhelonkin (based on the STM32 timer implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_TIMER_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_TIMER_H

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
    void BASETIMER_TEMPLATE_QUALIFIER::SetCounterMode(CounterMode mode)
    {
        _Regs()->CTLR1 = (_Regs()->CTLR1 & ~(TIM_DIR | TIM_CMS)) | static_cast<uint16_t>(mode);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetCounterValue(BASETIMER_TEMPLATE_QUALIFIER::Counter counter)
    {
        _Regs()->CNT = counter;
    }

    BASETIMER_TEMPLATE_ARGS
    typename BASETIMER_TEMPLATE_QUALIFIER::Counter BASETIMER_TEMPLATE_QUALIFIER::GetCounterValue()
    {
        return static_cast<Counter>(_Regs()->CNT);
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
        return static_cast<Counter>(_Regs()->PSC);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetPeriod(BASETIMER_TEMPLATE_QUALIFIER::Counter period)
    {
        _Regs()->ATRLR = period;
    }

    BASETIMER_TEMPLATE_ARGS
    typename BASETIMER_TEMPLATE_QUALIFIER::Counter BASETIMER_TEMPLATE_QUALIFIER::GetPeriod()
    {
        return static_cast<Counter>(_Regs()->ATRLR);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetPeriodAndUpdate(BASETIMER_TEMPLATE_QUALIFIER::Counter period)
    {
        _Regs()->ATRLR = period;
        _Regs()->SWEVGR = TIM_UG;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::EnableOnePulseMode()
    {
        _Regs()->CTLR1 |= TIM_OPM;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DisableOnePulseMode()
    {
        _Regs()->CTLR1 &= ~TIM_OPM;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Stop()
    {
        _Regs()->CTLR1 &= ~TIM_CEN;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::Start()
    {
        _Regs()->SWEVGR = TIM_UG;
        _Regs()->CTLR1 |= TIM_CEN;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::EnableInterrupt(Interrupt interrupt)
    {
        _Regs()->DMAINTENR |= static_cast<uint32_t>(interrupt);
        NVIC_EnableIRQ(_IRQNumber);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DisableInterrupt(Interrupt interrupt)
    {
        _Regs()->DMAINTENR &= ~static_cast<uint32_t>(interrupt);
    }

    BASETIMER_TEMPLATE_ARGS
    bool BASETIMER_TEMPLATE_QUALIFIER::IsInterrupt()
    {
        return (_Regs()->INTFR & TIM_UIF) != 0;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::ClearInterruptFlag()
    {
        _Regs()->INTFR = ~static_cast<uint32_t>(TIM_UIF);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::SetMasterMode(MasterMode mode)
    {
        _Regs()->CTLR2 = (_Regs()->CTLR2 & ~TIM_MMS) | static_cast<uint16_t>(mode);
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DmaRequestEnable()
    {
        _Regs()->DMAINTENR |= TIM_UDE;
    }

    BASETIMER_TEMPLATE_ARGS
    void BASETIMER_TEMPLATE_QUALIFIER::DmaRequestDisable()
    {
        _Regs()->DMAINTENR &= ~TIM_UDE;
    }

    #define ADVANCEDTIMER_TEMPLATE_ARGS template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
    #define ADVANCEDTIMER_TEMPLATE_QUALIFIER AdvancedTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>

    ADVANCEDTIMER_TEMPLATE_ARGS
    void ADVANCEDTIMER_TEMPLATE_QUALIFIER::EnableMainOutput()
    {
        _Regs()->BDTR |= TIM_MOE;
    }

    ADVANCEDTIMER_TEMPLATE_ARGS
    void ADVANCEDTIMER_TEMPLATE_QUALIFIER::DisableMainOutput()
    {
        _Regs()->BDTR &= ~TIM_MOE;
    }

    ADVANCEDTIMER_TEMPLATE_ARGS
    void ADVANCEDTIMER_TEMPLATE_QUALIFIER::SetRepetitionCounter(uint8_t value)
    {
        _Regs()->RPTCR = value;
    }

    ADVANCEDTIMER_TEMPLATE_ARGS
    uint8_t ADVANCEDTIMER_TEMPLATE_QUALIFIER::GetRepetitionCounter()
    {
        return static_cast<uint8_t>(_Regs()->RPTCR);
    }

    #undef BASETIMER_TEMPLATE_ARGS
    #undef BASETIMER_TEMPLATE_QUALIFIER
    #undef ADVANCEDTIMER_TEMPLATE_ARGS
    #undef ADVANCEDTIMER_TEMPLATE_QUALIFIER
}
#endif // ZHELE_PLATFORM_CH32_COMMON_IMPL_TIMER_H
