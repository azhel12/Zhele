/**
 * @file
 * Implement types list for C++20
 * 
 * @author Alexey Zhelonkin (based on VladislavS code)
 * @date 2023
 * @license FreeBSD
 */

#ifndef ZHELE_CONCEPTS_H
#define ZHELE_CONCEPTS_H

#include <concepts>
#include <type_traits>

namespace Zhele::TemplateUtils {
    template<typename T>
    concept enum_t = std::is_enum_v<T>;
}

#endif //! ZHELE_CONCEPTS_H