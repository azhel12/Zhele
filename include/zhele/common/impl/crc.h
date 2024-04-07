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
#include <type_traits>

namespace Zhele
{
    #if defined (CRC_POL_POL)
        inline void Crc::SetPolynomSize(PolynomSize polynomSize)
        {
            CRC->CR = (CRC->CR & ~CRC_CR_POLYSIZE_Msk) | (static_cast<uint32_t>(polynomSize));
        }

        inline void Crc::SetPolynom(auto polynom)
        {
            CRC->POL = polynom;
        }

        inline uint32_t Crc::CalculateCrc32(uint32_t polynom, const uint8_t* data, unsigned size)
        {
            SetPolynom(polynom);
            return CalculateCrc32(data, size);
        }
    #else

    #endif

    #if defined (CRC_INIT_INIT)
        inline void Crc::SetInitialValue(uint32_t initialValue)
        {
            CRC->INIT = initialValue;
        }
    #endif

        inline uint32_t Crc::GetPolynom()
        {
        #if defined (CRC_POL_POL)
            return CRC->POL;
        #else
            return 0x4c11db7;
        #endif
        }

        inline uint32_t Crc::GetInitialValue()
        {
        #if defined (CRC_INIT_INIT)
            return CRC->INIT;
        #else
            return 0xffffffff;
        #endif
        }

        inline uint32_t Crc::Write(auto chunk)
        {
            CRC->DR = chunk;

            return CRC->DR;
        }

        inline uint32_t Crc::Read()
        {
            return CRC->DR;
        }

        inline uint32_t Crc::CalculateCrc32(const uint8_t* data, unsigned size)
        {
            Reset();
            for(int i = 0; i < size / sizeof(uint32_t); ++i) {
                CRC->DR = *reinterpret_cast<const uint32_t*>(data);
                data += sizeof(uint32_t);
            }

            for(int i = 0; i < size % sizeof(uint32_t); ++i) {
                CRC->DR = data[i];
            }
        }

        inline void Crc::Reset()
        {
            CRC->CR |= CRC_CR_RESET;
            while(CRC->CR & CRC_CR_RESET) continue;
        }

        inline void Crc::SetIDR(std::remove_cv_t<decltype(CRC_TypeDef::IDR)> data)
        {
            CRC->IDR = data;
        }

        inline std::remove_cv_t<decltype(CRC_TypeDef::IDR)> Crc::GetIDR()
        {
            return CRC->IDR;
        }
}

#endif //! ZHELE_CRC_IMPL_COMMON_H