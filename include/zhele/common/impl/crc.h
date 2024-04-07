/**
 * @file
 * @brief Implements CRC32
 * @author Alezey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_CRC_IMPL_COMMON_H
#define ZHELE_CRC_IMPL_COMMON_H

#include <stdint.h>
#include <limits>
#include <type_traits>

namespace Zhele
{
#if defined (CRC_POL_POL)
    template<typename _Clock>
    inline void Crc32<_Clock>::SetPolynomSize(PolynomSize polynomSize)
    {
        CRC->CR = (CRC->CR & ~CRC_CR_POLYSIZE_Msk) | (static_cast<uint32_t>(polynomSize));
    }

    template<typename _Clock>
    inline void Crc32<_Clock>::SetPolynom(auto polynom)
    {
        CRC->POL = polynom;
    }

    template<typename _Clock>
    inline uint32_t Crc32<_Clock>::CalculateCrc32(uint32_t polynom, const uint8_t* data, unsigned size)
    {
        SetPolynom(polynom);
        return CalculateCrc32(data, size);
    }
#else

#endif

#if defined (CRC_INIT_INIT)
    template<typename _Clock>
    inline void Crc32<_Clock>::SetInitialValue(uint32_t initialValue)
    {
        CRC->INIT = initialValue;
    }
#endif

    template<typename _Clock>
    inline void Zhele::Crc32<_Clock>::Enable()
    {
        _Clock::Enable();

    #if defined (CRC_CR_REV_IN) && defined (CRC_CR_REV_OUT)
        CRC->CR |= (0b11 << CRC_CR_REV_IN_Pos) | CRC_CR_REV_OUT;
    #endif
    }

    template<typename _Clock>
    inline uint32_t Crc32<_Clock>::GetPolynom()
    {
    #if defined (CRC_POL_POL)
        return CRC->POL;
    #else
        return 0x4c11db7;
    #endif
    }

    template<typename _Clock>
    inline uint32_t Crc32<_Clock>::GetInitialValue()
    {
    #if defined (CRC_INIT_INIT)
        return CRC->INIT;
    #else
        return 0xffffffff;
    #endif
    }

    template<typename _Clock>
    inline uint32_t Crc32<_Clock>::CalculateCrc32(const uint8_t* data, unsigned size)
    {
        Reset();
        const uint32_t* data_aligned = reinterpret_cast<const uint32_t*>(data);

#if defined (CRC_CR_REV_IN) && defined (CRC_CR_REV_OUT)
        for (int i = 0; i < (size / sizeof(uint32_t)); ++i) {
            CRC->DR = data_aligned[i];
        }

        uint32_t result = CRC->DR;
        unsigned bits_remain = (size % sizeof(uint32_t)) * std::numeric_limits<uint8_t>::digits;
        if (bits_remain) {
            CRC->DR = CRC->DR;
            CRC->DR = ((data_aligned[size / sizeof(uint32_t)] & (0xFFFFFFFF >> (32 - bits_remain))) ^ result) << (32 - bits_remain);
            result = (result >> bits_remain) ^ CRC->DR;
        }
#else
        for (int i = 0; i < (size / sizeof(uint32_t)); ++i) {
            CRC->DR = __RBIT(data_aligned[i]);
        }


        uint32_t result = __RBIT(CRC->DR);
        unsigned bits_remain = (size % sizeof(uint32_t)) * std::numeric_limits<uint8_t>::digits;
        if (bits_remain) {
            CRC->DR = CRC->DR;
            CRC->DR = __RBIT((data_aligned[size / sizeof(uint32_t)] & (0xFFFFFFFF >> (32 - bits_remain))) ^ result) >> (32 - bits_remain);
            result = (result >> bits_remain) ^ __RBIT(CRC->DR);
        }
#endif

        return ~result;
    }

    template<typename _Clock>
    inline void Crc32<_Clock>::Reset()
    {
        CRC->CR |= CRC_CR_RESET;
        while(CRC->CR & CRC_CR_RESET) continue;
    }

    template<typename _Clock>
    inline void Crc32<_Clock>::SetIDR(std::remove_cv_t<decltype(CRC_TypeDef::IDR)> data)
    {
        CRC->IDR = data;
    }

    template<typename _Clock>
    inline std::remove_cv_t<decltype(CRC_TypeDef::IDR)> Crc32<_Clock>::GetIDR()
    {
        return CRC->IDR;
    }
}

#endif //! ZHELE_CRC_IMPL_COMMON_H