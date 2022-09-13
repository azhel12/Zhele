/*
 * @file
 * United header for iopins
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#if defined(STM32F0)
    #include "f0/iopins.h"
#endif
#if defined(STM32F1)
    #include "f1/iopins.h"
#endif
#if defined(STM32F4)
    #include "f4/iopins.h"
#endif
#if defined(STM32L4)
    #include "l4/iopins.h"
#endif