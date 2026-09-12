/**
 * @file
 * United header for SPI
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#ifndef ZHELE_SPI_H
#define ZHELE_SPI_H

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
#include "platform/stm32/spi.h"
#elif defined(ZHELE_PLATFORM_CH32)
#include "platform/ch32/spi.h"
#elif defined(ZHELE_PLATFORM_NIIET)
#include "platform/niiet/spi.h"
#else
#error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers."
#endif

#endif // ZHELE_SPI_H
