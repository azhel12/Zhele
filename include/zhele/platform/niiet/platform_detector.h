/**
 * @file
 * NIIET: detect platform and device from CMSIS / project macros.
 *
 * Defines:
 *   ZHELE_PLATFORM_NIIET          — any NIIET target
 *   ZHELE_NIIET_DEVICE_K1921VG015 — K1921VG015 (RISC-V SCR1, 3×16-pin GPIO ports)
 *   ZHELE_NIIET_DEVICE_K1921VG5T  — K1921VG5T  (RISC-V SCR4, motor-control line)
 *   ZHELE_NIIET_DEVICE_K1921VG7T  — K1921VG7T  (RISC-V SCR4, general purpose line)
 *   ZHELE_NIIET_SERIES_VGXT       — VG5T/VG7T share one peripheral IP generation
 *                                   (RCU with UARTCFG/SPICFG muxes, PL022 SPI with a
 *                                   single fused CR, AXI DMA). VG015 is the older one.
 *
 * Detection order:
 *   1. K1921VGxx compile-time define (e.g. -DK1921VG7T from CMake).
 *   2. __K1921VGxx_H header guard (set after #include <K1921VGxx.h>).
 *   3. Explicit ZHELE_PLATFORM_NIIET — allowed for testing / mocking.
 */
#ifndef ZHELE_PLATFORM_NIIET_PLATFORM_DETECTOR_H
#define ZHELE_PLATFORM_NIIET_PLATFORM_DETECTOR_H

#if defined(K1921VG015) || defined(__K1921VG015_H)
  #define ZHELE_PLATFORM_NIIET          1
  #define ZHELE_NIIET_DEVICE_K1921VG015 1
#elif defined(K1921VG5T) || defined(__K1921VG5T_H)
  #define ZHELE_PLATFORM_NIIET          1
  #define ZHELE_NIIET_DEVICE_K1921VG5T  1
  #define ZHELE_NIIET_SERIES_VGXT       1
#elif defined(K1921VG7T) || defined(__K1921VG7T_H)
  #define ZHELE_PLATFORM_NIIET          1
  #define ZHELE_NIIET_DEVICE_K1921VG7T  1
  #define ZHELE_NIIET_SERIES_VGXT       1
#elif defined(ZHELE_PLATFORM_NIIET)
  // Device / family must also be set explicitly when overriding.
#endif

#endif // ZHELE_PLATFORM_NIIET_PLATFORM_DETECTOR_H
