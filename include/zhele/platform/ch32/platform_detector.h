/**
 * @file
 * CH32: detect platform and family from WCH SDK / project macros.
 *
 * Defines:
 *   ZHELE_PLATFORM_CH32       — any CH32 target
 *   ZHELE_CH32_FAMILY_V0      — CH32V003 / CH32V00x  (RISC-V, 8-pin ports)
 *   ZHELE_CH32_FAMILY_V1      — CH32V103 / CH32V10x
 *   ZHELE_CH32_FAMILY_V2      — CH32V203 / CH32V20x
 *   ZHELE_CH32_FAMILY_V3      — CH32V303 / CH32V30x
 */
#pragma once

// wch_cmake sets CH32V0 (family) + CH32V00x (type); WCH SDK sets CH32V003/CH32V006/etc.
#if defined(CH32V0) || defined(CH32V00x) \
 || defined(CH32V003) || defined(CH32V003F4) || defined(CH32V003A4) || defined(CH32V003J4) \
 || defined(CH32V006) || defined(CH32V006K8U6) || defined(CH32V006K8R6) \
 || defined(CH32V006F8P6) || defined(CH32V006F8U6) || defined(CH32V006J8U6)
#  define ZHELE_PLATFORM_CH32    1
#  define ZHELE_CH32_FAMILY_V0   1
#elif defined(CH32V1) || defined(CH32V10x) || defined(CH32V103)
#  define ZHELE_PLATFORM_CH32    1
#  define ZHELE_CH32_FAMILY_V1   1
#elif defined(CH32V2) || defined(CH32V20x) || defined(CH32V203)
#  define ZHELE_PLATFORM_CH32    1
#  define ZHELE_CH32_FAMILY_V2   1
#elif defined(CH32V3) || defined(CH32V30x) || defined(CH32V303)
#  define ZHELE_PLATFORM_CH32    1
#  define ZHELE_CH32_FAMILY_V3   1
#elif defined(ZHELE_PLATFORM_CH32)
  // Family set explicitly without a WCH SDK macro — allowed for testing/mocking.
#endif


