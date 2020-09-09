#if defined (STM32F4)
    #include "../include/f4/clock.h"

namespace Zhele::Clock
{
    const static unsigned PllmBitFieldOffset = RCC_PLLCFGR_PLLM_Pos;
    const static unsigned PllmBitFieldLength = GetBitFieldLength<(RCC_PLLCFGR_PLLM_Msk >> RCC_PLLCFGR_PLLM_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllM, uint32_t, PllmBitFieldOffset, PllmBitFieldLength);

    const static unsigned PllnBitFieldOffset = RCC_PLLCFGR_PLLN_Pos;
    const static unsigned PllnBitFieldLength = GetBitFieldLength<(RCC_PLLCFGR_PLLN_Msk >> RCC_PLLCFGR_PLLN_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllN, uint32_t, PllnBitFieldOffset, PllnBitFieldLength);

    const static unsigned PllpBitFieldOffset = RCC_PLLCFGR_PLLP_Pos;
    const static unsigned PllpBitFieldLength = GetBitFieldLength<(RCC_PLLCFGR_PLLP_Msk >> RCC_PLLCFGR_PLLP_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllP, uint32_t, PllpBitFieldOffset, PllpBitFieldLength);

    const static unsigned PllqBitFieldOffset = RCC_PLLCFGR_PLLQ_Pos;
    const static unsigned PllqBitFieldLength = GetBitFieldLength<(RCC_PLLCFGR_PLLQ_Msk >> RCC_PLLCFGR_PLLQ_Pos)>;
    IO_BITFIELD_WRAPPER(RCC->PLLCFGR, PllQ, uint32_t, PllqBitFieldOffset, PllqBitFieldLength);

    ClockFrequenceT PllClock::SetClockFreq(ClockFrequenceT freq)
    {
        return 0;
    }

    ClockFrequenceT PllClock::GetDivider()
    {
        return PllM::Get();
    }

    void PllClock::SetDivider(uint8_t divider)
    {
        PllM::Set(divider);
    }

    ClockFrequenceT PllClock::GetMultipler()
    {
        return PllN::Get();
    }

    void PllClock::SetMultipler(uint8_t multiplier)
    {
        PllN::Set(multiplier);
    }  

    void PllClock::SelectClockSource(ClockSource clockSource)
    {
        RCC->CFGR = clockSource == External
            ? RCC->CFGR  | RCC_PLLCFGR_PLLSRC_HSE
            : RCC->CFGR  & ~RCC_PLLCFGR_PLLSRC;
    }

    PllClock::ClockSource PllClock::GetClockSource()
    {
        return RCC->CFGR & RCC_PLLCFGR_PLLSRC_HSE
            ? ClockSource::External
            : ClockSource::Internal;
    }

    ClockFrequenceT PllClock::GetGeneralOutputDivider()
    {
        return PllP::Get();
    }
    void PllClock::SetGeneralOutputDivider(uint8_t divider)
    {
        PllP::Set(divider);
    }
    ClockFrequenceT PllClock::GetUsbOutputDivider()
    {
        return PllQ::Get();
    }
    void PllClock::SetUsbOutputDivider(uint8_t divider)
    {
        PllQ::Set(divider);
    }
}

#endif
