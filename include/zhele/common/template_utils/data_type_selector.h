/**
 * @file
 * Implements "type factory" of needed length
 * 
 * @author Konstantin Chizhov
 * @date 2013
 * @license MIT
 */

#ifndef ZHELE_COMMON_TEMPLATE_UTILS_DATA_TYPE_SELECTOR_H
#define ZHELE_COMMON_TEMPLATE_UTILS_DATA_TYPE_SELECTOR_H

#include "type_list.h"

#include <bit>
#include <cstdint>

namespace Zhele::template_utils {
    /**
    * @brief Allows to select suitable unsigned integer type to cover necessary bit count
    * @details
    * 1..8 - uint_fast8_t
    * 9..16 - uint_fast16_t
    * 17..32 - uint_fast32_t
    * 33..64 - uint_fast64_t
    */
    template<uint8_t length>
    static consteval auto GetSuitableUnsignedType() {
        static_assert(length >= 1 && length <= 64, "Bit length must be in range 1..64");

        if constexpr (length <= 8) {
            return type_box<uint_fast8_t>{};
        } else if constexpr (length <= 16) {
            return type_box<uint_fast16_t>{};
        } else if constexpr (length <= 32) {
            return type_box<uint_fast32_t>{};
        } else {
            return type_box<uint_fast64_t>{};
        }
    }

    /**
    * @brief Allows to select suitable unsigned integer type to cover necessary length
    * @details
    * Picks the fast unsigned type able to hold values up to (and including) Size.
    * 1..255 - uint_fast8_t
    * 256..2^16 - 1 - uint_fast16_t
    * 2^16..2^32 - 1 - uint_fast32_t
    * 2^32..2^64 - 1 - uint_fast64_t
    *
    * @note Delegates to \ref GetSuitableUnsignedType: the number of bits needed to
    *       store Size is std::bit_width(Size) (at least 1 so that Size == 0 stays valid).
    */
    template<unsigned Size>
    class SuitableUnsignedTypeForLength {
        static constexpr uint8_t bits = Size == 0 ? 1 : std::bit_width(Size);
    public:
        using type = type_unbox<GetSuitableUnsignedType<bits>()>;
    };
}

#endif //! ZHELE_COMMON_TEMPLATE_UTILS_DATA_TYPE_SELECTOR_H
