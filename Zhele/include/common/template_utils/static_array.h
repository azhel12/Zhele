/**
 * @file
 * Implements static array.
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_STATICARRAY_H
#define ZHELE_STATICARRAY_H

#include "type_list.h"

namespace Zhele
{
    template<unsigned... Numbers>
    class UnsignedArray;

    /**
     * @brief Array length
     */
    template<typename...>
    class Length {};

    template<unsigned... Numbers>
    class Length<UnsignedArray<Numbers...>>
    {
    public:
        static const size_t value = sizeof...(Numbers);
    };

    /**
     * @brief Select number from array by index
     */
    template<unsigned, typename...>
    class GetNumber {};

    template<unsigned Head, unsigned... Tail>
    class GetNumber<0, UnsignedArray<Head, Tail...>>
    {
    public:
        static const unsigned value = Head;
    };

    template<unsigned Index, unsigned Head, unsigned... Tail>
    class GetNumber<Index, UnsignedArray<Head , Tail...>>
    {
        static_assert(Index < Length<UnsignedArray<Head, Tail...>>::value);
    public:
        static const unsigned value = GetNumber<Index - 1, UnsignedArray<Tail...>>::value;
    };
}

#endif //!ZHELE_STATICARRAY_H