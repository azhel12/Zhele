/**
 * @file
 * GPIO ports for CH32 — dispatches to the correct family implementation.
 */
#pragma once

// Run the family detector even if ZHELE_PLATFORM_CH32 was set externally
// (the root platform_detector.h skips it in that case).
#include "platform_detector.h"

#if defined(ZHELE_CH32_FAMILY_V0)
#  include "v0/ioports.h"
#elif defined(ZHELE_CH32_FAMILY_V1) || defined(ZHELE_CH32_FAMILY_V2) || defined(ZHELE_CH32_FAMILY_V3)
#  error "Zhele: CH32 V1/V2/V3 GPIO not yet implemented. Contributions welcome."
#else
#  error "Zhele: CH32 family not detected. Define CH32V003 (or the correct WCH device macro) or set ZHELE_CH32_FAMILY_V0 manually."
#endif


