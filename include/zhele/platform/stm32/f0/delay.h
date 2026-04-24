/**
 * @file
 * Implements delay for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#pragma once

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
    template<unsigned long ms, unsigned long CpuFreq = F_CPU>
    void delay_ms()
    {
        delay_us<ms * 1000, CpuFreq>();
    }

}
