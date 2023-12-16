/**
 * @file
 * United header for Flash
 *
 * @author Alexey Zhelonkin
 * @date 2020
 * @licence FreeBSD
 */

#if defined(STM32F0)
    #include "f0/flash.h"
#endif
#if defined(STM32F1)
    #include "f1/flash.h"
#endif
#if defined(STM32F4)
    #include "f4/flash.h"
#endif
#if defined(STM32L4)
    #include "l4/flash.h"
#endif