/**
 * @file
 * Implements binary stream
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_BINARY_STREAM_H
#define ZHELE_BINARY_STREAM_H

#include <cstdint>

namespace Zhele
{
    /// Endianness
    enum Endianness
    {
        BigEndian = 0, ///< Big endian
        LittleEndian = 1, ///< Little endian
        MixedEndian = 2 ///< Mixed endian
    };

    /**
     * @brief Implements binary stream
     * 
     * @tparam _Source Data source
     */
    template<typename _Source>
    class BinaryStream :public _Source
    {
    public:
        /**
         * @brief Construct a new Binary Stream object
         * 
         * @tparam Args Data source arguments types
         * @param args Data source arguments
         */
        template<typename... Args>
        BinaryStream(Args... args)
            : _Source(args...)
        {
        }

        /**
         * @brief Reads 4-bytes big-endian value
         * 
         * @return uint32_t Readed value
         */
        inline uint32_t ReadU32Be();

        /**
         * @brief Reads 4-bytes little-endian value
         * 
         * @return uint32_t Readed value
         */
        inline uint32_t ReadU32Le();

        /**
         * @brief Reads 2-bytes big-endian value
         * 
         * @return uint16_t Readed value
         */
        inline uint16_t ReadU16Be();

        /**
         * @brief Reads 2-bytes little-endian value
         * 
         * @return uint16_t Readed value
         */
        inline uint16_t ReadU16Le();

        /**
         * @brief Reads 1-byte value
         * 
         * @return uint8_t Readed value
         */
        inline uint8_t ReadU8(){ return _Source::Read(); }

        /**
         * @brief Reads 1-byte value
         * 
         * @return uint8_t Readed value
         */
        inline uint8_t Read(){ return _Source::Read(); }

        /**
         * @brief Writes 4-bytes big-endian value
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void WriteU32Be(uint32_t value);

        /**
         * @brief Writes 4-bytes little-endian value
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void WriteU32Le(uint32_t value);

        /**
         * @brief Writes 2-bytes big-endian value
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void WriteU16Be(uint16_t value);

        /**
         * @brief Writes 2-bytes little-endian value
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void WriteU16Le(uint16_t value);

        /**
         * @brief Writes 1-byte value
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void WriteU8(uint8_t value){ _Source::Write(value); }

        /**
         * @brief Writes 1-byte
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
         */
        inline void Write(uint8_t value){ _Source::Write(value); }
         
        /**
         * @brief Reads and discards specified number of bytes
         * 
         * @param bytes Bytes to ignore
         * @return uint8_t Last byte read
         */
        inline uint8_t Ignore(size_t bytes);

        /**
         * @brief Reads and discards specified number of bytes or until 'delim' byte is found
         * 
         * @param bytes Bytes to ignore
         * @param delim Delimeter value
         * @return uint8_t Last byte read
         */
        inline uint8_t Ignore(size_t bytes, uint8_t delim);

        /**
         * @brief Reads and discards specified number of bytes while read byte is eq to 'value'
         * 
         * @param bytes Bytes to ignore
         * @param value Delimeter value
         * @return uint8_t Last byte read
         */
        inline uint8_t IgnoreWhile(size_t bytes, uint8_t value);

        /**
         * @brief Read data from source to buffer
         * 
         * @tparam PtrType Receive buffer pointer type
         * 
         * @param buffer [out] Buffer pointer
         * @param size [in] Bytes to read
         */
        template<typename PtrType>
        inline void Read(PtrType buffer, size_t size)
        {
            for(size_t i = 0; i < size; ++i)
            {
                *buffer = _Source::Read();
                ++buffer;
            }
        }

        /**
         * @brief Read data from source to buffer async
         * 
         * @tparam PtrType Receive buffer pointer type
         * 
         * @param buffer [out] Buffer pointer
         * @param size [in] Bytes to read
         * @param callback [in, opt] Read complete callback
         * 
         */
        template<typename PtrType>
        inline void ReadAsync(PtrType buffer, size_t size, TransferCallback callback = nullptr)
        {
            _Source::ReadAsync(buffer, size, callback);
        }

        /**
         * @brief Writes data
         * 
         * @tparam PtrType Source buffer pointer type
         * @param buffer [in] Buffer pointer
         * @param size [in] Bytes to write
         */
        template<typename PtrType>
        inline void Write(PtrType buffer, size_t size)
        {
            for(size_t i = 0; i < size; ++i)
            {
                _Source::Write(*buffer);
                ++buffer;
            }
        }

        /**
         * @brief Writes data async
         * 
         * @tparam PtrType Source buffer pointer type
         * @param buffer [in] Buffer pointer
         * @param size [in] Bytes to write
         */
        template<typename PtrType>
        inline void WriteAsync(PtrType buffer, size_t size)
        {
            _Source::WriteAsync(buffer, size);
        }
    };
}

#include "impl/binary_stream.h"

#endif //! ZHELE_BINARY_STREAM_H