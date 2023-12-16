/**
 * @file
 * Implement DAC for stm32f4 series
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DAC_H
#define ZHELE_DAC_H

#include <stm32l4xx.h>
#include "../common/dac.h"

namespace Zhele
{
    /// Dac trigger
    enum class DacTrigger : uint8_t
    {
        Timer6Trgo = 0x00, ///< Timger 6 TRGO event
        Timer8Trgo = 0x01, ///< Timger 8 TRGO event
        Timer7Trgo = 0x02, ///< Timger 7 TRGO event
        Timer5Trgo = 0x03, ///< Timger 5 TRGO event
        Timer2Trgo = 0x04, ///< Timger 2 TRGO event
        Timer4Trgo = 0x05, ///< Timger 4 TRGO event
        Exti9 = 0x06, ///< External line 9
        Software = 0x07 ///< Software trigger
    };
}

#endif //! ZHELE_DAC_H