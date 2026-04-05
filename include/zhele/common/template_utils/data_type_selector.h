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

#include "mp"

#include <cstdint>
#include <type_traits>

namespace Zhele::TemplateUtils {
    /**
     * @brief Allows to select suitable unsigned integer type to cover necessary bit count
     * @details
     * 1..8 - uint_fast8_t
     * 9..16 - uint_fast16_t
     * 17..32 - uint_fast32_t
     * 33..64 - uint_fast64_t
     * 
     * @returns mp::meta of suitable unsigned integer type 
     */
    template<uint8_t length>
    static consteval auto GetSuitableUnsignedType() {
        constexpr mp::array types = {
            // 1 - uint_fast8_t
            mp::meta<uint_fast8_t>,
            // 2 - uint_fast16_t
            mp::meta<uint_fast16_t>,
            // 3-4 - uint_fast32_t
            mp::meta<uint_fast32_t>,
            mp::meta<uint_fast32_t>,
            // 5-8 - uint_fast64_t
            mp::meta<uint_fast64_t>,
            mp::meta<uint_fast64_t>,
            mp::meta<uint_fast64_t>,
            mp::meta<uint_fast64_t>
        };
        constexpr auto idx = (static_cast<std::size_t>(length) - 1u) / 8u;

        return types[idx];
    }

    /**
     * @brief Smallest fast unsigned type that can represent indices/lengths up to `Size` inclusive.
     */
    template<unsigned Size>
    struct SuitableUnsignedTypeForLength {
        using type = mp::type_of<GetSuitableUnsignedType<Size>()>;
    };
}

#endif // ZHELE_DATA_TYPE_SELECTOR_H