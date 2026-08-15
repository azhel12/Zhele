/**
 * @file
 * United header for SPI
 * 
 * @author Aleksei Zhelonkin
 * @date 2019
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_SPI_H
#define ZHELE_PLATFORM_STM32_SPI_H

#if defined(STM32F0)
    #include "f0/spi.h"
#endif
#if defined(STM32F1)
    #include "f1/spi.h"
#endif
#if defined(STM32F4)
    #include "f4/spi.h"
#endif
#if defined(STM32L4)
    #include "l4/spi.h"
#endif
#if defined(STM32G0)
    #include "g0/spi.h"
#endif
#if defined(STM32G4)
    #include "g4/spi.h"
#endif

#endif // ZHELE_PLATFORM_STM32_SPI_H
