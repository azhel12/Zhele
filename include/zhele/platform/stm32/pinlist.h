/**
 * @file
 * United header for pinlist
 * 
 * @author Aleksei Zhelonkin
 * @date 2019
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_PINLIST_H
#define ZHELE_PLATFORM_STM32_PINLIST_H

#if defined(STM32F0)
    #include "f0/pinlist.h"
#endif
#if defined(STM32F1)
    #include "f1/pinlist.h"
#endif
#if defined(STM32F4)
    #include "f4/pinlist.h"
#endif
#if defined(STM32L4)
    #include "l4/pinlist.h"
#endif
#if defined(STM32G0)
    #include "g0/pinlist.h"
#endif
#if defined(STM32H5)
    #include "h5/pinlist.h"
#endif

#endif // ZHELE_PLATFORM_STM32_PINLIST_H
