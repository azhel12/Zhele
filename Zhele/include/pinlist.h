 /**
 * @file
 * United header for pinlist
 * 
 * @author Aleksei Zhelonkin
 * @date 2019
 * @licence FreeBSD
 */

#if defined(STM32F0)
    #include <f0/pinlist.h>
#endif
#if defined(STM32F1)
    #include <f1/pinlist.h>
#endif
#if defined(STM32F4)
    #include <f4/pinlist.h>
#endif
#if defined(STM32L4)
    #include <l4/pinlist.h>
#endif