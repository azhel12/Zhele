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
    template<int, typename...>
    class GetNumber {};

    template<unsigned Head, unsigned... Tail>
    class GetNumber<-1, UnsignedArray<Head, Tail...>>
    {
    public:
        static const unsigned value = 0;
    };

    template<unsigned Head, unsigned... Tail>
    class GetNumber<0, UnsignedArray<Head, Tail...>>
    {
    public:
        static const unsigned value = Head;
    };

    template<int Index, unsigned Head, unsigned... Tail>
    class GetNumber<Index, UnsignedArray<Head , Tail...>>
    {
        static_assert(Index < Length<UnsignedArray<Head, Tail...>>::value);
    public:
        static const unsigned value = GetNumber<Index - 1, UnsignedArray<Tail...>>::value;
    };

    /**
     * @brief Select number from array by index. Non-static variant.
     */
    template<typename>
    class GetNumberRuntime
    {
    public:
        /**
         * @brief Returns value in array.
         * 
         * @param [in] index Index
         * 
         * @returns Value with given index.
         */
        static uint8_t Get(uint8_t index);
    };

    template<unsigned... Numbers>
    class GetNumberRuntime<UnsignedArray<Numbers ...>>
    {
        const static uint8_t _altFuncNumbers[sizeof...(Numbers)];
    public:
        static uint8_t Get(uint8_t index) {return _altFuncNumbers[index];};
    };

    template<unsigned... Numbers>
    const uint8_t GetNumberRuntime<UnsignedArray<Numbers ...>>::_altFuncNumbers[sizeof... (Numbers)] = {Numbers ...};
}

#endif //!ZHELE_STATICARRAY_H