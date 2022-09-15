#if defined (STM32L4)
    #include "../include/l4/clock.h"

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

    void PllClock::SetDivider(ClockFrequenceT divider)
    {
        PllM::Set(divider);
    }

    ClockFrequenceT PllClock::GetMultipler()
    {
        return PllN::Get();
    }

    void PllClock::SetMultiplier(ClockFrequenceT multiplier)
    {
        PllN::Set(multiplier);
    }  

    void PllClock::SelectClockSource(ClockSource clockSource)
    {
        RCC->PLLCFGR = clockSource == External
            ? RCC->PLLCFGR  | RCC_PLLCFGR_PLLSRC_HSE
            : RCC->PLLCFGR  & ~RCC_PLLCFGR_PLLSRC;
    }

    PllClock::ClockSource PllClock::GetClockSource()
    {
        return RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC_HSE
            ? ClockSource::External
            : ClockSource::Internal;
    }

    PllClock::SystemOutputDivider PllClock::GetSystemOutputDivider()
    {
        return static_cast<PllClock::SystemOutputDivider>(PllP::Get());
    }
    void PllClock::SetSystemOutputDivider(PllClock::SystemOutputDivider divider)
    {
        PllP::Set(static_cast<uint8_t>(divider));
    }
    ClockFrequenceT PllClock::GetUsbOutputDivider()
    {
        return PllQ::Get();
    }
    void PllClock::SetUsbOutputDivider(ClockFrequenceT divider)
    {
        PllQ::Set(divider);
    }
}

#endif