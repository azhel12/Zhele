/**
 * @file
 * Implements fixed string which allows use string
 * literals as template arguments
 * 
 * Idea: https://vector-of-bool.github.io/2021/10/22/string-templates.html
 * 
 * @author Alexey Zhelonkin
 * @date 2021
 * @license MIT
 */

#pragma once

#include <type_traits>

namespace Zhele::template_utils
{

    template <unsigned _Length>
    struct fixed_string
    {
        const static unsigned Length = _Length;
        const static unsigned Size = _Length;
        char Text[_Length + 1] = {};
    };

    template <unsigned _Length>
    struct fixed_string_16
    {
        const static unsigned Length = _Length;
        const static unsigned Size = _Length * 2;
        char16_t Text[_Length + 2] = {};
    };

    template <unsigned _Length>
    fixed_string(const char (&str)[_Length]) -> fixed_string<_Length - 1>;

    template <unsigned _Length>
    fixed_string_16(const char16_t (&str)[_Length]) -> fixed_string_16<_Length - 1>;

    constexpr Zhele::template_utils::fixed_string_16 EmptyFixedString16(u"");
}

