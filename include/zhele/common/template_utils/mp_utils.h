/**
 * @file
 * Contains utility functions for mp library
 * 
 * @author Aleksei Zhelonkin
 * @date 2026
 * @licence FreeBSD
 */

#ifndef ZHELE_COMMON_TEMPLATE_UTILS_MP_UTILS_H
#define ZHELE_COMMON_TEMPLATE_UTILS_MP_UTILS_H

#include "mp"

#include <algorithm>
#include <ranges>
#include <type_traits>

namespace mp {
    /**
     * @brief Counts unique metas in a range
     * 
     * @param metas Range of metas to count unique metas in
     * @returns Count of unique metas in the range
     */
    template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, mp::info>
    static constexpr auto unique_count(R&& metas) {
        auto metas_sorted = metas;
        std::ranges::sort(metas_sorted);

        auto [ret, _] = std::ranges::unique(metas_sorted);
        return std::distance(metas_sorted.begin(), ret);
    }

    /**
     * @brief Sorts metas in a range
     * 
     * @param metas Range of metas to sort
     * 
     * @returns Sorted range of metas
     */
    template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, mp::info>
    static constexpr auto sorted(R&& metas) {
        auto out = metas;
        std::ranges::sort(out);
    
        return out;
    }

    /**
     * @brief Removes duplicates from a range of metas
     * 
     * @param metas Range of metas to remove duplicates from
     * 
     * @returns mp::vector with unique metas
     */
    template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, mp::info>
    static constexpr auto unique(R&& metas) {
        mp::vector<mp::info> out;
        for (auto&& m : metas) {
            if (std::ranges::find(out, m) == out.end()) {
                out.push_back(m);
            }
        }
        return out;
    }
}

#endif // ZHELE_COMMON_TEMPLATE_UTILS_MP_UTILS_H