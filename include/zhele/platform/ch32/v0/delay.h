/**
 * @file
 * Busy-loop delay for CH32V00x series.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_DELAY_H
#define ZHELE_PLATFORM_CH32_V0_DELAY_H

// CH32V003 default clock is 48 MHz HSI; a consumer may override F_CPU.
#ifndef F_CPU
#  define F_CPU 48000000u
#endif

namespace Zhele
{
    static const unsigned DelayInitInstructionsCount = 14;
    static const unsigned InstructionsPerCycle = 4;

    template<unsigned long us, unsigned long CpuFreq = F_CPU>
    void delay_us()
    {
        static const unsigned int loops = (CpuFreq / 1000000u * us - DelayInitInstructionsCount) / InstructionsPerCycle;
        for (unsigned int i = 0; i < loops; ++i) {
            __asm__ volatile ("nop");
        }
    }

    template<unsigned long ms, unsigned long CpuFreq = F_CPU>
    void delay_ms()
    {
        delay_us<ms * 1000, CpuFreq>();
    }
}

#endif // ZHELE_PLATFORM_CH32_V0_DELAY_H
