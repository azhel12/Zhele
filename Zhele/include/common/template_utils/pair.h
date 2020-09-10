/**
 * @file
 * Implements Pair of types.
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_PAIR_H
#define ZHELE_PAIR_H

namespace Zhele
{
    template<typename _Key, typename _Value>
    class Pair
    {
    public:
        using Key = _Key;
        using Value = _Value;
    };
}

#endif //!ZHELE_PAIR_H