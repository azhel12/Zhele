/**
 * @file
 * @brief Implements DAC
 * @date 2022
 * @author Aleksei Zhelonkin
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_IMPL_DAC_H
#define ZHELE_PLATFORM_STM32_COMMON_IMPL_DAC_H

namespace Zhele::Private
{
    #define DAC_TEMPLATE_ARGS template <typename _Regs, typename _ClockCtrl, uint8_t _Channel>
    #define DAC_TEMPLATE_QUALIFIER DacBase<_Regs, _ClockCtrl, _Channel>

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::Init()
    {
        _ClockCtrl::Enable();
        SelectHighFrequencyMode();
    }

    DAC_TEMPLATE_ARGS
    template <typename Trigger>
    void DAC_TEMPLATE_QUALIFIER::Init(Trigger trigger)
    {
        _ClockCtrl::Enable();
        SelectHighFrequencyMode();
        _Regs()->CR |= (DAC_CR_TEN1
                    | (static_cast<uint8_t>(trigger) << DAC_CR_TSEL1_Pos)
                ) << (_Channel * ChannelOffset);
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::Enable()
    {
        _Regs()->CR |= DAC_CR_EN1 << (_Channel * ChannelOffset);
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::Disable()
    {
        _Regs()->CR &= ~(DAC_CR_EN1 << (_Channel * ChannelOffset));
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::EnableBuffer()
    {
#if defined (DAC_CR_BOFF1)
        _Regs()->CR &= ~(DAC_CR_BOFF1 << (_Channel * ChannelOffset));
#else
        // MODE = 000: normal mode, connected to external pin with buffer enabled
        _Regs()->MCR &= ~(DAC_MCR_MODE1_1 << (_Channel * ChannelOffset));
#endif
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::DisableBuffer()
    {
#if defined (DAC_CR_BOFF1)
        _Regs()->CR |= (DAC_CR_BOFF1 << (_Channel * ChannelOffset));
#else
        // MODE = 010: normal mode, connected to external pin with buffer disabled
        _Regs()->MCR |= (DAC_MCR_MODE1_1 << (_Channel * ChannelOffset));
#endif
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::SelectHighFrequencyMode()
    {
#if defined (DAC_MCR_HFSEL)
        // High frequency interface mode depends on AHB clock: > 160 MHz, > 80 MHz or below
        auto ahbFreq = _ClockCtrl::ClockFreq();
        _Regs()->MCR = (_Regs()->MCR & ~DAC_MCR_HFSEL)
            | (ahbFreq > 160000000u ? DAC_MCR_HFSEL_1 : (ahbFreq > 80000000u ? DAC_MCR_HFSEL_0 : 0));
#endif
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::EnableNoiseWaveGeneration(WaveAmplitude amplitude)
    {
        _Regs()->CR = (_Regs()->CR & ~((DAC_CR_WAVE1_Msk | DAC_CR_MAMP1_Msk) << (_Channel * ChannelOffset)))
            | (((static_cast<uint32_t>(WaveGeneration::Noise) << DAC_CR_WAVE1_Pos)
                | (static_cast<uint32_t>(amplitude) << DAC_CR_MAMP1_Pos)
                ) << (_Channel * ChannelOffset));
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::EnableTriangleWaveGeneration(WaveAmplitude amplitude)
    {
        _Regs()->CR = (_Regs()->CR & ~((DAC_CR_WAVE1_Msk | DAC_CR_MAMP1_Msk) << (_Channel * ChannelOffset)))
            | (((static_cast<uint32_t>(WaveGeneration::Triangle) << DAC_CR_WAVE1_Pos)
                | (static_cast<uint32_t>(amplitude) << DAC_CR_MAMP1_Pos)
                ) << (_Channel * ChannelOffset));
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::DisableWaveGeneration()
    {
        _Regs()->CR &= ~(DAC_CR_WAVE1_Msk << (_Channel * ChannelOffset));
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::WriteU8(uint8_t data)
    {
        if constexpr (_Channel == 0)
            _Regs()->DHR8R1 = data;
        else
            _Regs()->DHR8R2 = data;
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::Write(uint16_t data)
    {
        if constexpr (_Channel == 0)
            _Regs()->DHR12R1 = data;
        else
            _Regs()->DHR12R2 = data;
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::WriteLeftAligned(uint16_t data)
    {
        if constexpr (_Channel == 0)
            _Regs()->DHR12L1 = data;
        else
            _Regs()->DHR12L2 = data;
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::CauseSoftwareTrigger()
    {
        _Regs()->SWTRIGR = 1 << _Channel;
    }
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_IMPL_DAC_H