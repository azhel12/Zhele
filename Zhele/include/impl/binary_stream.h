/**
 * @file
 * Implements binary stream
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_BINARY_STREAM_IMPL_H
#define ZHELE_BINARY_STREAM_IMPL_H

namespace Zhele
{
    template<typename _Source>
    uint32_t BinaryStream<_Source>::ReadU32Be()
    {
        uint32_t result = uint32_t(_Source::Read()) << 24;
        result |= uint32_t(_Source::Read()) << 16;
        result |= uint32_t(_Source::Read()) << 8;
        result |= uint32_t(_Source::Read()) ;
        return result;
    }

    template<typename _Source>
    uint32_t BinaryStream<_Source>::ReadU32Le()
    {
        uint32_t result = uint32_t(_Source::Read());
        result |= uint32_t(_Source::Read()) << 8;
        result |= uint32_t(_Source::Read()) << 16;
        result |= uint32_t(_Source::Read()) << 24;
        return result;
    }

    template<typename _Source>
    uint16_t BinaryStream<_Source>::ReadU16Be()
    {
        uint16_t result = uint16_t(_Source::Read()) << 8;
        result |= uint16_t(_Source::Read());
        return result;
    }

    template<typename _Source>
    uint16_t BinaryStream<_Source>::ReadU16Le()
    {
        uint16_t result = uint16_t(_Source::Read());
        result |= uint16_t(_Source::Read()) << 8;
        return result;
    }

    template<typename _Source>
    void BinaryStream<_Source>::WriteU32Be(uint32_t value)
    {
        _Source::Write(uint8_t(value >> 24));
        _Source::Write(uint8_t(value >> 16));
        _Source::Write(uint8_t(value >> 8));
        _Source::Write(uint8_t(value));
    }

    template<typename _Source>
    void BinaryStream<_Source>::WriteU32Le(uint32_t value)
    {
        _Source::Write(uint8_t(value));
        _Source::Write(uint8_t(value >> 8));
        _Source::Write(uint8_t(value >> 16));
        _Source::Write(uint8_t(value >> 24));
    }

    template<typename _Source>
    void BinaryStream<_Source>::WriteU16Be(uint16_t value)
    {
        _Source::Write(uint8_t(value >> 8));
        _Source::Write(uint8_t(value));
    }

    template<typename _Source>
    void BinaryStream<_Source>::WriteU16Le(uint16_t value)
    {
        _Source::Write(uint8_t(value));
        _Source::Write(uint8_t(value >> 8));
    }

    template<typename _Source>
    uint8_t BinaryStream<_Source>::Ignore(size_t bytes)
    {
        uint8_t value = 0;
        for(size_t i = 0; i < bytes; ++i)
        {
            value = _Source::Read();
        }
        return value;
    }

    template<typename _Source>
    uint8_t BinaryStream<_Source>::Ignore(size_t bytes, uint8_t delim)
    {
        uint8_t value = 0;
        for(size_t i = 0; i < bytes; ++i)
        {
            value = _Source::Read();
            if(value == delim)
                break;
        }
        return value;
    }

    template<typename _Source>
    uint8_t BinaryStream<_Source>::IgnoreWhile(size_t bytes, uint8_t expected)
    {
        uint8_t value = expected;
        for(size_t i = 0; i < bytes; ++i)
        {
            value = _Source::Read();
            if(value != expected)
                break;
        }
        return value;
    }
}

#endif //! ZHELE_BINARY_STREAM_IMPL_H