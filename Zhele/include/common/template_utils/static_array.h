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

#include <cstddef>

namespace Zhele::TemplateUtils
{
    template<auto... Numbers>
    class NonTypeTemplateArray;

    template<auto... Numbers>
    class Length<NonTypeTemplateArray<Numbers...>>
    {
    public:
        static const size_t value = sizeof...(Numbers);
    };
    /**
     * @brief Select number from array by index
     */
    template<int, typename...>
    class GetNonTypeValueByIndex {};
    template<auto Head, auto... Tail>
    class GetNonTypeValueByIndex<-1, NonTypeTemplateArray<Head, Tail...>>
    {
    public:
        static const auto value = 0;
    };
    template<auto Head, auto... Tail>
    class GetNonTypeValueByIndex<0, NonTypeTemplateArray<Head, Tail...>>
    {
    public:
        static const auto value = Head;
    };
    template<int Index, auto Head, auto... Tail>
    class GetNonTypeValueByIndex<Index, NonTypeTemplateArray<Head , Tail...>>
    {
        static_assert(Index < Length<NonTypeTemplateArray<Head, Tail...>>::value);
    public:
        static const auto value = GetNonTypeValueByIndex<Index - 1, NonTypeTemplateArray<Tail...>>::value;
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

    template<auto... Numbers>
    class GetNumberRuntime<NonTypeTemplateArray<Numbers ...>>
    {
        const static uint8_t _numbers[sizeof...(Numbers)];
    public:
        static uint8_t Get(uint8_t index) {return _numbers[index];};
    };

    template<auto... Numbers>
    const uint8_t GetNumberRuntime<NonTypeTemplateArray<Numbers ...>>::_numbers[sizeof... (Numbers)] = {Numbers ...};

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