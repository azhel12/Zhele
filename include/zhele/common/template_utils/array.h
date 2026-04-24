/**
 * @file
 * Implement typed arrays for C++20
 *
 * @author X-Ray
 * @date 2026
 * @license MIT
 */

#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace Zhele {

inline namespace Arrays {

// Alias &  CTAD for std::array of uint8_t
template <std::size_t N>
struct ArrayU8 : std::array<uint8_t, N> { };

ArrayU8(std::integral auto... ints) -> ArrayU8<sizeof...(ints)>;

// Alias & CTAD for std::array of uint16_t
template <std::size_t N>
struct ArrayU16 : std::array<uint16_t, N> { };

ArrayU16(std::integral auto... ints) -> ArrayU16<sizeof...(ints)>;

// Alias & CTAD for std::array of uint32_t
template <std::size_t N>
struct ArrayU32 : std::array<uint32_t, N> { };

ArrayU32(std::integral auto... ints) -> ArrayU32<sizeof...(ints)>;

// Alias & CTAD for std::array of uint64_t
template <std::size_t N>
struct ArrayU64 : std::array<uint64_t, N> { };

ArrayU64(std::integral auto... ints) -> ArrayU64<sizeof...(ints)>;

// Alias &  CTAD for std::array of uint8_t
template <std::size_t N>
struct ArrayS8 : std::array<uint8_t, N> { };

ArrayS8(std::integral auto... ints) -> ArrayS8<sizeof...(ints)>;

// Alias & CTAD for std::array of uint16_t
template <std::size_t N>
struct ArrayS16 : std::array<uint16_t, N> { };

ArrayS16(std::integral auto... ints) -> ArrayS16<sizeof...(ints)>;

// Alias & CTAD for std::array of uint32_t
template <std::size_t N>
struct ArrayS32 : std::array<uint32_t, N> { };

ArrayS32(std::integral auto... ints) -> ArrayS32<sizeof...(ints)>;

// Alias & CTAD for std::array of uint64_t
template <std::size_t N>
struct ArrayS64 : std::array<uint64_t, N> { };

ArrayS64(std::integral auto... ints) -> ArrayS64<sizeof...(ints)>;

}

} // namespace Zhele::Arrays
