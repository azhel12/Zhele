/**
 * @file
 * Clock control for NIIET — selects the device headers, then pulls in the
 * peripheral clock helpers.
 */
#ifndef ZHELE_PLATFORM_NIIET_CLOCK_H
#define ZHELE_PLATFORM_NIIET_CLOCK_H

// Run the device detector even if ZHELE_PLATFORM_NIIET was set externally.
#include "platform_detector.h"

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
  #include <K1921VG015.h>
#elif defined(ZHELE_NIIET_DEVICE_K1921VG5T)
  #include <K1921VG5T.h>
#elif defined(ZHELE_NIIET_DEVICE_K1921VG7T)
  #include <K1921VG7T.h>
#else
  #error "Zhele: NIIET device not detected. Define K1921VG015 / K1921VG5T / K1921VG7T (compiler flag) or include the CMSIS device header before Zhele headers."
#endif

#include "common/clock.h"

#endif // ZHELE_PLATFORM_NIIET_CLOCK_H
