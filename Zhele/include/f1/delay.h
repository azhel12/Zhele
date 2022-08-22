/**
 * @file
 * Implements delay for stm32f1 series
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
     * @brief Microseconds delay
     * 
     * @details
     * I had continuous fail when was writing one-wire protocol.
     * It was because this connection method VERY exacting for delays.
     * STM32 MCUs start with CortexM3-based contains 32bit DWT register,
     * which count MCU ticks. So, it's possible to measure us intervals with
     * this register, or measure some code and repeat it.
     * Now second way is implemented.
     * Known, that call delay_us requires 14 + 10 * N ticks,
     * where N is loop counter. It follows loop value equals to
     * (CpuFreq / 1000000u * us - 48) / 13;
     * 
     * @todo 
     * It has been measured only for stm32f103c8t6 (and for -Os compiler key) and MUST be written better in future
     */
    const static unsigned DelayInitInstructionsCount = 14;
    const static unsigned InstructionsPerCycle = 10;
    template<unsigned long us, unsigned long CpuFreq = F_CPU>
    void delay_us()
    {
        static const unsigned int loops = (CpuFreq / 1000000u * us - DelayInitInstructionsCount) / InstructionsPerCycle;
        for (unsigned int i = 0; i < loops; ++i) __asm("nop");
    }

}
#endif //! ZHELE_DELAY_H