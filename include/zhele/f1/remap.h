 /**
 * @file
 * Contains class remap GPIO on stm32f1
 * 
 * STM32F1 mcu has specific scheme for remap gpio pins.
 * There is no AFR register in GPIO_TypeDef struct for each port, but
 * two registers (MAPR/MAPR2). So, STM32F1 does not assume remap pins and
 * assume remap pins set for each module instead.
 * 
 * For example, you can remap Timer2 (all channels at the same time).
 * 
 * For compability I tried enable "SelectPins" method for periphery (for timer, for example),
 * but there is some limitations. For example, Timer realization allows select pins for each channel
 * via SelectPins. Inside this method for F1 I edit MAPR register AND REMAP ALSO OTHER CHANNELS!
 * So if you select Pa6 for Tim3Ch1 and after that select Pb5 for Tim3Ch2 then you also select Pb4 for Tim3Ch1.
 * You must remember this feature and use Remap class for custom remap.
 * 
 * @author Aleksei Zhelonkin
 * @date 2020
 * @licence FreeBSD
 */

#ifndef ZHELE_REMAP_H
#define ZHELE_REMAP_H

#include <stm32f1xx.h>
#include "../common/ioreg.h"

#include "clock.h"

namespace Zhele::IO
{
    namespace Private
    {
        template<typename _ClockRegs>
        class PeriphRemapBitField{};

        #define DECLARE_PERIPH_REMAP(CLOCK_REGS, REMAP_BITFIELD) \
        template<> \
        class PeriphRemapBitField<CLOCK_REGS> \
        { \
        public: \
            using BitField = REMAP_BITFIELD; \
        }; \

        // Timers Remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Timer1RemapBitField, AFIO_MAPR_TIM1_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Tim1Clock, Timer1RemapBitField)

        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Timer2RemapBitField, AFIO_MAPR_TIM2_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Tim2Clock, Timer2RemapBitField)

        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Timer3RemapBitField, AFIO_MAPR_TIM3_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Tim3Clock, Timer3RemapBitField)
    #if defined (TIM4)
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Timer4RemapBitField, AFIO_MAPR_TIM4_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Tim4Clock, Timer4RemapBitField)
    #endif

        // Usarts remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart1RemapBitField, AFIO_MAPR_USART1_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Usart1Clock, Usart1RemapBitField);

        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart2RemapBitField, AFIO_MAPR_USART2_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Usart2Clock, Usart2RemapBitField);

        #if defined (USART3)
            DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart3RemapBitField, AFIO_MAPR_USART3_REMAP)
            DECLARE_PERIPH_REMAP(Zhele::Clock::Usart3Clock, Usart3RemapBitField);
        #endif


        // SPI remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Spi1RemapBitField, AFIO_MAPR_SPI1_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::Spi1Clock, Spi1RemapBitField);
        #if defined(SPI3)
            DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Spi3RemapBitField, AFIO_MAPR_SPI3_REMAP)
            DECLARE_PERIPH_REMAP(Zhele::Clock::Spi3Clock, Spi3RemapBitField);
        #endif

        // I2C remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, I2c1RemapBitField, AFIO_MAPR_I2C1_REMAP)
        DECLARE_PERIPH_REMAP(Zhele::Clock::I2c1Clock, I2c1RemapBitField)

        template<typename Clock>
        using PeriphRemap = typename Private::PeriphRemapBitField<Clock>::BitField;

        // SWJ remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, SwjRemapBitField, AFIO_MAPR_SWJ_CFG)

        template<typename Clock>
        using PeriphRemap = typename Private::PeriphRemapBitField<Clock>::BitField;
    }

    using Timer1Remap = Private::PeriphRemap<Zhele::Clock::Tim1Clock>;
    using Timer2Remap = Private::PeriphRemap<Zhele::Clock::Tim2Clock>;
    using Timer3Remap = Private::PeriphRemap<Zhele::Clock::Tim3Clock>;
#if defined (TIM4)
    using Timer4Remap = Private::PeriphRemap<Zhele::Clock::Tim4Clock>;
#endif
    using Usart1Remap = Private::PeriphRemap<Zhele::Clock::Usart1Clock>;
    using Usart2Remap = Private::PeriphRemap<Zhele::Clock::Usart2Clock>;

#if defined (USART3)
    using Usart3Remap = Private::PeriphRemap<Zhele::Clock::Usart3Clock>;
#endif
    using Spi1Remap = Private::PeriphRemap<Zhele::Clock::Spi1Clock>;
#if defined(SPI3)
    using Spi3Remap = Private::PeriphRemap<Zhele::Clock::Spi3Clock>;
#endif

    using I2c1Remap = Private::PeriphRemap<Zhele::Clock::I2c1Clock>;

    using SwjRemap = Private::SwjRemapBitField;
} // namespace Zhele::IO


#endif // !ZHELE_REMAP_H