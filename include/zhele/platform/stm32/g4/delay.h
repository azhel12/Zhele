/**
 * @file
 * Implements delay for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_DELAY_H
#define ZHELE_PLATFORM_STM32_G4_DELAY_H

#if !defined (F_CPU)
    #warning F_CPU not defined. Will be used F_CPU equals 16 MHz (HSI16).
    #define F_CPU 16000000UL
#endif

namespace Zhele
{
    /**
     * Copy from F1
     */
    const static unsigned DelayInitInstructionsCount = 14;
    const static unsigned InstructionsPerCycle = 10;
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
#endif //! ZHELE_PLATFORM_STM32_G4_DELAY_H
