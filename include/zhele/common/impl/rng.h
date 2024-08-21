/**
 * @file
 * Contains RNG methods defintion
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_RNG_IMPL_COMMON_H_
#define ZHELE_RNG_IMPL_COMMON_H_

namespace Zhele {
    void Rng::Init() {
        Zhele::Clock::RngClock::Enable();
        if((RNG->CR & RNG_CR_RNGEN) == 0)
        {
            RNG->CR |= RNG_CR_RNGEN;
        }
    }
    
    std::optional<uint32_t> Rng::Next() {
        unsigned timeout = _gen_timeout;
        while((RNG->SR & RNG_SR_DRDY) == 0 && --timeout){}
        if(timeout != 0) {
            return RNG->DR;
        }
        return std::nullopt;
    }
    
    std::optional<uint32_t> Rng::Next(uint32_t lowerBound, uint32_t upperBound) {
        auto random = Next();
        if(auto random = Next()) {
            return (lowerBound + (random.value() % (upperBound - lowerBound)));
        }
        return std::nullopt;
    }

    bool Rng::NextBytes(uint8_t* data, size_t size)
    {
        size_t sizeAligned = size & ~0x3u;

        for(size_t i = 0; i < sizeAligned; i += 4) {
            auto random = Next();
            if(!random)
                return false;

            data[i + 0] = static_cast<uint8_t>(random.value());
            data[i + 1] = static_cast<uint8_t>(random.value() >> 8);
            data[i + 2] = static_cast<uint8_t>(random.value()>> 16);
            data[i + 3] = static_cast<uint8_t>(random.value() >> 24);
        }

        auto random = Next();
        if(!random)
            return false;

        for(size_t i = sizeAligned; i < size; i++) {
            data[i] = uint8_t(random.value());
            random.value() >>= 8;
        }

        return true;
    }

    bool Rng::IsOk() {
        return (RNG->SR & (RNG_SR_CECS | RNG_SR_SECS)) == 0;
    }
} 

#endif //! ZHELE_RNG_IMPL_COMMON_H_