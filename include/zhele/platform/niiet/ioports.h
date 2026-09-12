/**
 * @file
 * GPIO ports for NIIET — dispatches to the correct device implementation.
 */
#ifndef ZHELE_PLATFORM_NIIET_IOPORTS_H
#define ZHELE_PLATFORM_NIIET_IOPORTS_H

// Run the device detector even if ZHELE_PLATFORM_NIIET was set externally.
#include "platform_detector.h"

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
  #include "k1921vg015/ioports.h"
#elif defined(ZHELE_NIIET_DEVICE_K1921VG5T)
  #include "k1921vg5t/ioports.h"
#elif defined(ZHELE_NIIET_DEVICE_K1921VG7T)
  #include "k1921vg7t/ioports.h"
#else
  #error "Zhele: NIIET device not detected. Define K1921VG015 / K1921VG5T / K1921VG7T (compiler flag) or include the CMSIS device header before Zhele headers."
#endif

#endif // ZHELE_PLATFORM_NIIET_IOPORTS_H
