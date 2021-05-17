/**
 * @file
 * Implements "type factory" of needed length
 * 
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_DATA_TYPE_SELECTOR_H
#define ZHELE_DATA_TYPE_SELECTOR_H

#include "type_list.h"
#include <stdint.h>

namespace Zhele::TemplateUtils
{
    /**
    * @brief Allows to select suitable unsigned integer type to cover necessary bit count
    * @details
    * 1..8 - uint_fast8_t
    * 9..16 - uint_fast16_t
    * 16..32 - uint_fast32_t
    * 33..64 - uint_fast64_t
    */
    template<uint8_t Length>
    class SuitableUnsignedType
    {
        using unsignedTypes = TypeList<uint_fast8_t, uint_fast16_t, uint_fast32_t, uint_fast16_t>;
    public:
        using type = typename GetType<(Length - 1) / 8, unsignedTypes>::type;
    };

    /**
    * @brief Allows to select suitable unsigned integer type to cover necessary length
    * @details
    * 1..255 - uint_fast8_t
    * 256..2^16 - 1 - uint_fast16_t
    * 2^16... - uint_fast32_t
    */
    template<unsigned Size>
    class SuitableUnsignedTypeForLength
    {
        using unsignedTypes = TypeList<uint_fast8_t, uint_fast16_t, uint_fast32_t, uint_fast16_t>;
    public:
        using type = std::conditional_t<Size <= 0xff,
            uint_fast8_t,
            std::conditional_t<Size <= 0xffff,
                uint_fast16_t,
                uint_fast32_t
            >>;
    };
}

#endif //! ZHELE_DATA_TYPE_SELECTOR_H
