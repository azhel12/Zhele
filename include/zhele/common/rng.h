/**
 * @file
 * Implements random module
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_RNG_COMMON_H_
#define ZHELE_RNG_COMMON_H_

#include <zhele/clock.h>

#include <cstdint>
#include <cstddef>
#include <optional>

namespace Zhele {

    class Rng {
        static constexpr unsigned _gen_timeout = 40 * 4;
    public:
        /**
         * @brief Initialize RNG module
         * 
         * @par Returns
         *  Nothing
         */
        static inline void Init();

        /**
         * @brief Generates one random number
         * 
         * @returns Random number from range [lowerBound, upperBound) or nothing if error
         */
        static inline std::optional<uint32_t> Next();

        /**
         * @brief Generates random number in given range exclude upperBound
         * 
         * @param lowerBound Range lower bound
         * @param upperBound Range upper bound
         * 
         * @returns Random number from range [lowerBound, upperBound) or nothing if error
         */
        static inline std::optional<uint32_t> Next(uint32_t lowerBound, uint32_t upperBound);

        /**
         * @brief Generates random data in given buffer
         * 
         * @param [out] data Data buffer
         * @param [in] size Bytes to generate
         * 
         * @retval true Generate OK
         * @retval false Generate fails (timeout)
         */
        static inline bool NextBytes(uint8_t* data, size_t size);

        /**
         * @brief Returns RNG module OK status
         * 
         * @retval true Module OK (no seed/clock error)
         * @retval false Module FAIL (seed or clock error)
         */
        static inline bool IsOk();
    };
}

#include "impl/rng.h"

#endif //! ZHELE_RNG_COMMON_H_