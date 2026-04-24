/**
 * @file
 * GPIO ports for NIIET — dispatches to the correct device implementation.
 */
#pragma once

// Run the device detector even if ZHELE_PLATFORM_NIIET was set externally.
#include "platform_detector.h"

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
  #include "k1921vg015/ioports.h"
#else
  #error "Zhele: NIIET device not detected. Define K1921VG015 (compiler flag) or include <K1921VG015.h> before Zhele headers."
#endif


