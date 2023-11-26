/**
 * @file
 * United header for ioports
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#if defined(STM32F0)
    #include "f0/ioports.h"
#endif
#if defined(STM32F1)
    #include "f1/ioports.h"
#endif
#if defined(STM32F4)
    #include "f4/ioports.h"
#endif
#if defined(STM32L4)
    #include "l4/ioports.h"
#endif