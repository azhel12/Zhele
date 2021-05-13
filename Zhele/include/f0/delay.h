/**
 * @file
 * Implements delay for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_DELAY_H
#define ZHELE_DELAY_H

namespace Zhele
{
    /**
     * Copied from F1
     */
    const static unsigned DelayInitInstructionsCount = 14;
    const static unsigned InstructionsPerCycle = 10;
    template<unsigned long us, unsigned long CpuFreq = F_CPU>
    void delay_us()
    {
        static const unsigned int loops = (CpuFreq / 1000000u * us - DelayInitInstructionsCount) / InstructionsPerCycle;
        for (unsigned int i = 0; i < loops; ++i) ;
    }
}
#endif //! ZHELE_DELAY_H