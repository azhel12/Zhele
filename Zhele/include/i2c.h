/**
 * @file
 * United header for I2C
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */
#if defined(STM32F0)
    #include <f0/i2c.h>
#endif
#if defined(STM32F1)
    #include <f1/i2c.h>
#endif
#if defined(STM32F4)
    #include <f4/i2c.h>
#endif
#if defined(STM32L4)
    #include <l4/i2c.h>
#endif