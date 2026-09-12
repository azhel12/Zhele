/**
 * @file
 * Busy-loop delays for NIIET devices.
 *
 * Same template API as the STM32 port (delay_us / delay_ms). There is no
 * cycle-accurate calibration here — the loop is a coarse core-cycle count with a
 * single nop body so the compiler keeps it. F_CPU defaults to the post-SystemInit
 * core clock of the selected device (K1921VG015: PLL0 = 50 MHz, K1921VG5T/VG7T:
 * PLL = 100 MHz); override -DF_CPU=<Hz> if you run a different system clock.
 */
#ifndef ZHELE_PLATFORM_NIIET_DELAY_H
#define ZHELE_PLATFORM_NIIET_DELAY_H

#include "platform_detector.h"

#ifndef F_CPU
#  if defined(ZHELE_NIIET_SERIES_VGXT)
#    define F_CPU 100000000u
#  else
#    define F_CPU 50000000u
#  endif
#endif

namespace Zhele
{
    // Assume ~3 core cycles per (volatile) loop iteration.
    static const unsigned long DelayCyclesPerLoop = 3u;

    template<unsigned long us, unsigned long CpuFreq = F_CPU>
    void delay_us()
    {
        static const unsigned long loops = (CpuFreq / 1000000u * us) / DelayCyclesPerLoop;
        // The volatile nop is a side effect, so the loop is not optimised away.
        for (unsigned long i = 0; i < loops; ++i)
            __asm volatile ("nop");
    }

    template<unsigned long ms, unsigned long CpuFreq = F_CPU>
    void delay_ms()
    {
        delay_us<ms * 1000u, CpuFreq>();
    }
}

#endif // ZHELE_PLATFORM_NIIET_DELAY_H
