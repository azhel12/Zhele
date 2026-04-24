/**
 * @file
 * @brief Template functions for enum bitwise operations
 *
 * @details
 * DECLARE_VIEWS_IOTA is used to declare incrementable_traits
 * to use enum type in to std::views::iota(Enum::Null, Enum::Size)
 *
 * @author Konstantin Chizhov / X-Ray
 * @date 2026
 * @license MIT
 */

#pragma once

#include <concepts>
#include <utility>

namespace Zhele {

inline namespace EnumOps {

template <typename E> concept Enum = std::is_enum_v<E>;

using std ::integral;

#if __cpp_lib_to_underlying >= 202102L
using std::to_underlying;
#else
template <Enum E>
[[nodiscard]] constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
#endif

template <Enum E>
[[nodiscard]] constexpr auto operator+(E left) noexcept { return to_underlying(left); }

template <Enum E>
[[nodiscard]] constexpr E& operator++(E& e) noexcept { return e = static_cast<E>(+e + 1); }

template <Enum E>
[[nodiscard]] constexpr E operator++(E& e, int) noexcept { return std::exchange(e, static_cast<E>(+e + 1)); }

template <Enum E>
[[nodiscard]] constexpr E& operator--(E& e) noexcept { return e = static_cast<E>(+e - 1); }

template <Enum E>
[[nodiscard]] constexpr E operator--(E& e, int) noexcept { return std::exchange(e, static_cast<E>(+e - 1)); }

#define DECLARE_VIEWS_IOTA(ENUM)                                        \
    template <>                                                         \
    struct std::incrementable_traits<ENUM> {                            \
        using difference_type = make_signed_t<underlying_type_t<ENUM>>; \
    };

// Enum OP Enum
template <Enum L, Enum R>
[[nodiscard]] constexpr L operator&(L left, R right) noexcept { return static_cast<L>(+left & +right); }

template <Enum L, Enum R>
[[nodiscard]] constexpr L operator^(L left, R right) noexcept { return static_cast<L>(+left ^ +right); }

template <Enum L, Enum R>
[[nodiscard]] constexpr L operator|(L left, R right) noexcept { return static_cast<L>(+left | +right); }

template <Enum E>
[[nodiscard]] constexpr E operator~(E left) noexcept { return static_cast<E>(~+left); }

// Enum OP integral
template <Enum L, integral R>
[[nodiscard]] constexpr L operator&(L left, R right) noexcept { return static_cast<L>(+left & right); }

template <Enum L, integral R>
[[nodiscard]] constexpr L operator^(L left, R right) noexcept { return static_cast<L>(+left ^ right); }

template <Enum L, integral R>
[[nodiscard]] constexpr L operator|(L left, R right) noexcept { return static_cast<L>(+left | right); }

// integral OP Enum
template <integral L, Enum R>
[[nodiscard]] constexpr R operator&(L left, R right) noexcept { return static_cast<R>(left & +right); }

template <integral L, Enum R>
[[nodiscard]] constexpr R operator^(L left, R right) noexcept { return static_cast<R>(left ^ +right); }

template <integral L, Enum R>
[[nodiscard]] constexpr R operator|(L left, R right) noexcept { return static_cast<R>(left | +right); }

// Enum OP= Enum
template <Enum L, Enum R>
[[nodiscard]] constexpr const L& operator&=(L& left, R right) noexcept { return left = left & right; }

template <Enum L, Enum R>
[[nodiscard]] constexpr const L& operator^=(L& left, R right) noexcept { return left = left ^ right; }

template <Enum L, Enum R>
[[nodiscard]] constexpr const L& operator|=(L& left, R right) noexcept { return left = left | right; }

// Enum OP= integral
template <Enum L, integral R>
[[nodiscard]] constexpr const L& operator&=(L& left, R right) noexcept { return left = left & right; }

template <Enum L, integral R>
[[nodiscard]] constexpr const L& operator^=(L& left, R right) noexcept { return left = left ^ right; }

template <Enum L, integral R>
[[nodiscard]] constexpr const L& operator|=(L& left, R right) noexcept { return left = left | right; }

} // namespace EnumOps

template <Enum E>
constexpr bool HasAllFlags(E value, E flags) noexcept { return (+value & +flags) == +flags; }

template <Enum E>
constexpr bool HasAnyFlag(E value, E flags) noexcept { return (+value & +flags) != 0; }

} // namespace Zhele

#if TEST
namespace Test {

using namespace Zhele::EnumOps;

enum class E : signed char {
    A,
    B,
    C,
    D
};

static_assert((E::B & E::C) == E::A);
static_assert((E::B ^ E::C) == E::D);
static_assert((E::B | E::C) == E::D);

static_assert((E::B & 2) == E::A);
static_assert((E::B ^ 2) == E::D);
static_assert((E::B | 2) == E::D);

static_assert((1 & E::C) == E::A);
static_assert((1 ^ E::C) == E::D);
static_assert((1 | E::C) == E::D);

static_assert(~E::A == static_cast<E>(-1));
static_assert(Zhele::HasAllFlags(E::D, E::B | E::C));
static_assert(Zhele::HasAnyFlag(E::D, E::C));

static_assert([](E e) consteval { return e &= 2; }(E::B) == E::A);
static_assert([](E e) consteval { return e ^= 2; }(E::B) == E::D);
static_assert([](E e) consteval { return e |= 2; }(E::B) == E::D);

static_assert([](E e) consteval { return e++; }(E::B) == E::B);
static_assert([](E e) consteval { return e--; }(E::B) == E::B);
static_assert([](E e) consteval { return ++e; }(E::B) == E::C);
static_assert([](E e) consteval { return --e; }(E::B) == E::A);
static_assert([](E e) consteval { auto _ = e++; return e; }(E::B) == E::C);
static_assert([](E e) consteval { auto _ = e--; return e; }(E::B) == E::A);

} // namespace Test

#endif
