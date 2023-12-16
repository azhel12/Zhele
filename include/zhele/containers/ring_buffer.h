/**
 * @file
 * Implements ring buffer.
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_RINGBUFFER_H
#define ZHELE_RINGBUFFER_H

#include "../common/template_utils/data_type_selector.h"

#include <atomic>
#include <type_traits>

namespace Zhele::Containers
{
    namespace Private
    {
        /**
         * @brief Implements ringbuffer with size equals to power of 2.
         * It is slightly faster than RingBuffer, but limited to sizes.
         * 
         * @tparam _Size Size
         * @tparam _DataType Data type
         * @tparam _Atomic Atomic
         */
        template<unsigned _Size, typename _DataType>
        class RingBufferPO2
        {
            static_assert((_Size & (_Size - 1)) == 0);//_Size must be a power of 2
            using Atomic = std::atomic<typename Zhele::TemplateUtils::SuitableUnsignedTypeForLength<_Size>::type>;
        public:
            using size_type = typename Zhele::TemplateUtils::SuitableUnsignedTypeForLength<_Size>::type;
            using reference = _DataType&;
            using const_reference = const _DataType&;

            /**
            * @brief Constructor.
            * 
            * @par Returns
            *   Nothing
            */
            RingBufferPO2();

            /**
             * @brief Returns capacity
             * 
             * @returns Buffer capacity
             */
            size_type capacity() const;

            /**
            * @brief Returns count of elements in the buffer
            * 
            * @returns Count of elements
            */
            size_type size() const;

            /**
            * @brief Check for emptiness
            * 
            * @retval true Buffer is empty
            * @retval false Buffer is not empty
            */
            bool empty() const;

            /**
            * @brief Check for fullnes
            * 
            * @retval true Buffer is full
            * @retval false Buffer is not full
            */
            bool full() const;

            /**
            * @brief Find out the value of the first element
            *
            * @returns Reference to element
            */
            reference front();

            /**
            * @brief Find out the value of the first element
            *
            * @returns Const reference to element
            */
            const_reference front() const;

            /**
            * @brief Find out the value of the last element 
            *
            * @returns Reference to element
            */
            reference back();

            /**
            * @brief Find out the value of the last element
            *
            * @returns Const reference to element
            */
            const_reference back() const;

            /**
            * @brief Add an item to the end.
            *
            * @param [in] value Value  
            * 
            * @retval false Buffer is full
            * @retval true Buffer is not full
            */
            bool push_back(const _DataType& x);

            /**
            * @brief Add DataType() an item to the end
            *
            * @retval false Buffer is full
            * @retval true Buffer buffer is not full
            */
            bool push_back();

            /**
            * @brief Retrieves the first element
            *
            * @retval false If is empty
            * @retval true If isn't empty
            */
            bool pop_front();

            /**
            * @brief Clear the buffer
            * 
            * @par Returns
            *   Nothing
            */
            void clear();

            /**
            * @brief Operator overload []
            * 
            * @param [in] index Index
            * 
            * @returns Reference to element
            */
            inline reference operator[] (size_type index);

            /**
            * @brief Operator overload []
            * 
            * @param [in] index Index
            * 
            * @returns Const reference to element
            */
            inline const_reference operator[] (size_type index)const;

        private:
            _DataType* data();
            const _DataType* data() const;

            unsigned _data[(sizeof(_DataType) * (_Size + 1) - 1) / sizeof(unsigned)];
            
            Atomic _writeCount;
            Atomic _readCount;

            static constexpr Atomic _mask = _Size - 1;
        };
        
        template<unsigned _Size, typename _DataType = uint8_t>
        class RingBuffer
        {
            using Atomic = std::atomic<typename Zhele::TemplateUtils::SuitableUnsignedTypeForLength<_Size>::type>;
        public:
            using size_type = typename Zhele::TemplateUtils::SuitableUnsignedTypeForLength<_Size>::type;
            using reference = _DataType&;
            using const_reference = const _DataType&;

            /**
             * @brief Constructor
             * 
             * @par Returns
             *  Nothing
             */
            RingBuffer();

            /**
             * @brief Returns capacity
             * 
             * @returns Buffer capacity
             */
            size_type capacity() const;

            /**
            * @brief Returns count of elements in the buffer
            * 
            * @returns Count of elements
            */
            size_type size() const;

            /**
            * @brief Check for emptiness
            * 
            * @retval true Buffer is empty
            * @retval false Buffer is not empty
            */
            bool empty() const;

            /**
            * @brief Check for fullnes
            * 
            * @retval true Buffer is full
            * @retval false Buffer is not full
            */
            bool full() const;

            /**
            * @brief Find out the value of the first element
            *
            * @returns Reference to element
            */
            reference front();

            /**
            * @brief Find out the value of the first element
            *
            * @returns Const reference to element
            */
            const_reference front() const;

            /**
            * @brief Find out the value of the last element 
            *
            * @returns Reference to element
            */
            reference back();

            /**
            * @brief Find out the value of the last element
            *
            * @returns Const reference to element
            */
            const_reference back() const;

            /**
            * @brief Add an item to the end.
            *
            * @param [in] value Value  
            * 
            * @retval false Buffer is full
            * @retval true Buffer is not full
            */
            bool push_back(const _DataType& x);

            /**
            * @brief Add DataType() an item to the end
            *
            * @retval false Buffer is full
            * @retval true Buffer buffer is not full
            */
            bool push_back();

            /**
            * @brief Retrieves the first element
            *
            * @retval false If is empty
            * @retval true If isn't empty
            */
            bool pop_front();

            /**
            * @brief Clear the buffer
            * 
            * @par Returns
            *   Nothing
            */
            void clear();

            /**
            * @brief Operator overload []
            * 
            * @param [in] index Index
            * 
            * @returns Reference to element
            */
            inline reference operator[] (size_type index);

            /**
            * @brief Operator overload []
            * 
            * @param [in] index Index
            * 
            * @returns Const reference to element
            */
            inline const_reference operator[] (size_type index)const;

        private:
            _DataType* data();
            const _DataType* data() const;

            unsigned _data[(sizeof(_DataType) * (_Size + 1) - 1) / sizeof(unsigned)];
            Atomic _count;
            Atomic _first;
            Atomic _last;
        };
    } // namespace Private

    template<unsigned _Size, typename _DataType>
    using RingBuffer = std::conditional_t<((_Size & (_Size - 1)) == 0),
        typename Private::RingBufferPO2<_Size, _DataType>,
        typename Private::RingBuffer<_Size, _DataType> >;
} // namespace Zhele::Containers

#include "impl/ring_buffer.h"


#endif //! ZHELE_RINGBUFFER_H