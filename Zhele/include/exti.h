/**
 * @file
 * United header for EXTI
 *
 * @author Alexey Zhelonkin
 * @date 2019
 * @licence FreeBSD
 */

#if defined(STM32F0)
    #include <f0/exti.h>
#endif
#if defined(STM32F1)
    #include <f1/exti.h>
#endif
#if defined(STM32F4)
    #include <f4/exti.h>
#endif
#if defined(STM32L4)
    #include <l4/exti.h>
#endif