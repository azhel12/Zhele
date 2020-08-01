/**
 * @file
 * @brief Implement flash
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_COMMON_H
#define ZHELE_FLASH_COMMON_H

#include <stdint.h>

namespace Zhele
{
    class Flash
	{
    public:
        static void ConfigureFrequence(uint32_t frequence);
    };
}

#endif //! ZHELE_FLASH_COMMON_H