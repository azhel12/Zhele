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
#else
  #error "Zhele: NIIET device not detected. Define K1921VG015 (compiler flag) or include <K1921VG015.h> before Zhele headers."
#endif

#include "common/clock.h"

#endif // ZHELE_PLATFORM_NIIET_CLOCK_H
