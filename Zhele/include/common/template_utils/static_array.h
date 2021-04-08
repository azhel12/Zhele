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

namespace Zhele::TemplateUtils
{
    template<uint32_t... Numbers>
    class UnsignedArray;

    template<uint32_t... Numbers>
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
    template<uint32_t Head, uint32_t... Tail>
    class GetNumber<-1, UnsignedArray<Head, Tail...>>
    {
    public:
        static const uint32_t value = 0;
    };
    template<uint32_t Head, uint32_t... Tail>
    class GetNumber<0, UnsignedArray<Head, Tail...>>
    {
    public:
        static const uint32_t value = Head;
    };
    template<int Index, uint32_t Head, uint32_t... Tail>
    class GetNumber<Index, UnsignedArray<Head , Tail...>>
    {
        static_assert(Index < Length<UnsignedArray<Head, Tail...>>::value);
    public:
        static const uint32_t value = GetNumber<Index - 1, UnsignedArray<Tail...>>::value;
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

    template<uint32_t... Numbers>
    class GetNumberRuntime<UnsignedArray<Numbers ...>>
    {
        const static uint8_t _numbers[sizeof...(Numbers)];
    public:
        static uint8_t Get(uint8_t index) {return _numbers[index];};
    };

    template<uint32_t... Numbers>
    const uint8_t GetNumberRuntime<UnsignedArray<Numbers ...>>::_numbers[sizeof... (Numbers)] = {Numbers ...};

    /**
     * @brief Static array of int8_t.
     */
    template<int8_t... Numbers>
    class Int8_tArray;
    template<typename, int8_t>
    /**
     * @brief Inserts value in back.
     */
    class Int8_tArray_InsertBack {};
    template<int8_t Value, int8_t... Numbers>
    class Int8_tArray_InsertBack<Int8_tArray<Numbers...>, Value>
    {
    public:
        using type = Int8_tArray<Numbers..., Value>;
    };
}

#endif //!ZHELE_STATICARRAY_H