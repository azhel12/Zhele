/**
 * @file
 * @brief Implements DAC
 * @date 2022
 * @author Aleksei Zhelonkin
 * @license FreeBSD
 */

#ifndef ZHELE_DAC_IMPL_COMMON_H
#define ZHELE_DAC_IMPL_COMMON_H

namespace Zhele::Private
{
    #define DAC_TEMPLATE_ARGS template <typename _Regs, typename _ClockCtrl, uint8_t _Channel>
    #define DAC_TEMPLATE_QUALIFIER DacBase<_Regs, _ClockCtrl, _Channel>

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::Init()
    {
        _ClockCtrl::Enable();
    }

    DAC_TEMPLATE_ARGS
    template <typename Trigger>
    void DAC_TEMPLATE_QUALIFIER::Init(Trigger trigger)
    {
        _ClockCtrl::Enable();
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
        _Regs()->CR &= ~(DAC_CR_BOFF1 << (_Channel * ChannelOffset));
    }

    DAC_TEMPLATE_ARGS
    void DAC_TEMPLATE_QUALIFIER::DisableBuffer()
    {
        _Regs()->CR |= (DAC_CR_BOFF1 << (_Channel * ChannelOffset));
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

#endif //! ZHELE_DAC_IMPL_COMMON_H