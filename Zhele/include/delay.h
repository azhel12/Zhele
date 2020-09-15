/**
 * @file
 * United header for delay
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#if defined(STM32F0)
    #include "f0/delay.h"
#endif
#if defined(STM32F1)
    #include "f1/delay.h"
#endif
#if defined(STM32F4)
    #include "f4/delay.h"
#endif