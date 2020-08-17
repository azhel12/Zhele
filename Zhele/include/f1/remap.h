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

namespace Zhele::IO
{
    namespace Private
    {
        template<typename _Regs>
        class PeriphRemapBitField{};

        // Timers Remap
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Tim1RemapBitField, AFIO_MAPR_TIM1_REMAP)
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Tim2RemapBitField, AFIO_MAPR_TIM2_REMAP)
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Tim3RemapBitField, AFIO_MAPR_TIM3_REMAP)
        DECLARE_IO_BITFIELD_WRAPPER(AFIO->MAPR, Tim4RemapBitField, AFIO_MAPR_TIM4_REMAP)

        class Timer1;
        class Timer2;
        class Timer3;
        class Timer4;
        template<>
        class PeriphRemapBitField<Timer1>
        {
        public:
            using BitField = Tim1RemapBitField;
        };
        template<>
        class PeriphRemapBitField<Timer2>
        {
        public:
            using BitField = Tim2RemapBitField;
        };
        template<>
        class PeriphRemapBitField<Timer3>
        {
        public:
            using BitField = Tim3RemapBitField;
        };
        template<>
        class PeriphRemapBitField<Timer4>
        {
        public:
            using BitField = Tim4RemapBitField;
        };

        template<typename Periph>
        using PeriphRemap = typename Private::PeriphRemapBitField<Periph>::BitField;
    }

    using Timer1Remap = Private::PeriphRemap<Private::Timer1>;
    using Timer2Remap = Private::PeriphRemap<Private::Timer2>;
    using Timer3Remap = Private::PeriphRemap<Private::Timer3>;
    using Timer4Remap = Private::PeriphRemap<Private::Timer4>;
} // namespace Zhele::IO


#endif // !ZHELE_REMAP_H