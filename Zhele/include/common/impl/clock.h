/**
 * @file
 * Clock methods implementation.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_IMPL_COMMON_H
#define ZHELE_CLOCK_IMPL_COMMON_H

#if !defined  (HSE_VALUE) 
    #warning HSE_VALUE not defined. Will be used HSE_VALUE equals 8 MHZ.
    #define HSE_VALUE    ((uint32_t)8000000)
#endif //! HSE_VALUE

#if !defined  (HSI_VALUE)
    #warning HSI_VALUE not defined. Will be used HSI_VALUE equals 8 MHZ.
    #define HSI_VALUE    ((uint32_t)8000000)
#endif //! HSI_VALUE

namespace Zhele::Clock
{
    template <typename _Regs>
    bool ClockBase<_Regs>::EnableClockSource(unsigned turnMask, unsigned waitReadyMask)
    {
        uint32_t timeoutCounter = 0;
        _Regs::Or(turnMask);
        while(((_Regs::Get() & waitReadyMask) == 0) && (timeoutCounter < ClockStartTimeout))
        {
            timeoutCounter++;
        }
        return (_Regs::Get() & waitReadyMask) != 0;  
    }

    template <typename _Regs>
    bool ClockBase<_Regs>::DisableClockSource(unsigned turnMask, unsigned waitReadyMask)
    {
        uint32_t timeoutCounter = 0;
        _Regs::And(~turnMask);
        while(((_Regs::Get() & waitReadyMask) != 0) && (timeoutCounter < ClockStartTimeout))
        {
            timeoutCounter++;
        }
        return (_Regs::Get() & waitReadyMask) == 0;
    }

    template<typename _SrcClock, typename _PrescalerBitField>
    ClockFrequenceT BusClock<_SrcClock, _PrescalerBitField>::SrcClockFreq()
    {
        return _SrcClock::ClockFreq();
    }

    template<typename _SrcClock, typename _PrescalerBitField>
    template<typename PrescalerType>
    void BusClock<_SrcClock, _PrescalerBitField>::SetPrescaler(PrescalerType prescaler)
    {
        _PrescalerBitField::Set(static_cast<ClockFrequenceT>(prescaler));
    }

    template<typename _Reg, unsigned _Mask, typename _ClockSrc>
    void ClockControl<_Reg, _Mask, _ClockSrc>::Enable()
    {
        _Reg::Or(_Mask);
    }

    template<typename _Reg, unsigned _Mask, typename _ClockSrc>
    void ClockControl<_Reg, _Mask, _ClockSrc>::Disable()
    {
        _Reg::And(~_Mask);
    }

#if defined (RCC_CSR_LSION)
    constexpr ClockFrequenceT LsiClock::SrcClockFreq()
    {
        return 32768;
    }

    constexpr ClockFrequenceT LsiClock::GetDivider() { return 1; }

    constexpr ClockFrequenceT LsiClock::GetMultipler() { return 1; }

    constexpr ClockFrequenceT LsiClock::ClockFreq()
    {
        return SrcClockFreq();
    }
#endif
}

#endif //! ZHELE_CLOCK_IMPL_COMMON_H