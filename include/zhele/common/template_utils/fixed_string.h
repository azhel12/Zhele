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

#ifndef ZHELE_COMMON_TEMPLATE_UTILS_FIXEDSTRING_H
#define ZHELE_COMMON_TEMPLATE_UTILS_FIXEDSTRING_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace Zhele::template_utils {

    /**
     * @brief Compile-time fixed-capacity string usable as a non-type template argument.
     *
     * @tparam CharT Character type (char, char16_t, ...)
     * @tparam N Character count (excluding the null terminator)
     */
    template <typename CharT, std::size_t N>
    struct basic_fixed_string {
        using value_type = CharT;
        using view_type = std::basic_string_view<CharT>;

        std::array<CharT, N + 1> Text {};

        static constexpr std::size_t Length = N;
        static constexpr std::size_t Size = N * sizeof(CharT);

        constexpr basic_fixed_string() = default;
        constexpr basic_fixed_string(const CharT (&str)[N + 1]) { std::copy_n(str, N + 1, Text.begin()); }

        static constexpr std::size_t size() { return N; }
        constexpr const CharT* data() const { return Text.data(); }
        constexpr const CharT* begin() const { return Text.data(); }
        constexpr const CharT* end() const { return Text.data() + N; }
        constexpr CharT operator[](std::size_t index) const { return Text[index]; }

        constexpr operator view_type() const { return view_type(Text.data(), N); }

        constexpr bool operator==(const basic_fixed_string&) const = default;
    };

    template <typename CharT, std::size_t M>
    basic_fixed_string(const CharT (&str)[M]) -> basic_fixed_string<CharT, M - 1>;

    template <std::size_t N>
    using fixed_string = basic_fixed_string<char, N>;

    template <std::size_t N>
    using fixed_string_16 = basic_fixed_string<char16_t, N>;

    constexpr fixed_string_16 EmptyFixedString16(u"");
}

#endif //!ZHELE_COMMON_TEMPLATE_UTILS_FIXEDSTRING_H
