/**
 * @file
 * @brief Implements delay for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_DELAY_H
#define ZHELE_PLATFORM_CH32_X0_DELAY_H

#ifndef F_CPU
#  define F_CPU 48000000u
#endif

namespace Zhele
{
    static const unsigned DelayInitInstructionsCount = 14;
    // Measured at 48 MHz (2 flash wait states)
    static const unsigned InstructionsPerCycle = 12;

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

#endif // ZHELE_PLATFORM_CH32_X0_DELAY_H
