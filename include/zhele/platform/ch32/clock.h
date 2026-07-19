/**
 * @file
 * Clock for CH32 — dispatches to the correct family implementation.
 */
#ifndef ZHELE_PLATFORM_CH32_CLOCK_H
#define ZHELE_PLATFORM_CH32_CLOCK_H

#include "platform_detector.h"

#if defined(ZHELE_CH32_FAMILY_V0)
#  include "v0/clock.h"
#elif defined(ZHELE_CH32_FAMILY_V2)
#  include "v2/clock.h"
#elif defined(ZHELE_CH32_FAMILY_V1) || defined(ZHELE_CH32_FAMILY_V3)
#  error "Zhele: CH32 V1/V3 clock not yet implemented. Contributions welcome."
#else
#  error "Zhele: CH32 family not detected. Define CH32V003 (or the correct WCH device macro) or set ZHELE_CH32_FAMILY_V0 manually."
#endif

#endif // ZHELE_PLATFORM_CH32_CLOCK_H
