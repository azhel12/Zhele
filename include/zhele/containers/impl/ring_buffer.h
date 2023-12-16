/**
 * @file
 * Ring buffer methods implementation.
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_RINGBUFFER_IMPL_H
#define ZHELE_RINGBUFFER_IMPL_H

#include <atomic>

namespace Zhele::Containers::Private
{
    #define RINGBUFFERPO2_TEMPLATE_ARGS template<unsigned _Size, typename _DataType>
    #define RINGBUFFERPO2_TEMPLATE_QUALIFIER RingBufferPO2<_Size, _DataType>

    RINGBUFFERPO2_TEMPLATE_ARGS
    RINGBUFFERPO2_TEMPLATE_QUALIFIER::RingBufferPO2() :_writeCount(0),  _readCount(0)
    {
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    typename RINGBUFFERPO2_TEMPLATE_QUALIFIER::size_type RINGBUFFERPO2_TEMPLATE_QUALIFIER::capacity() const
    {
        return _Size;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    typename RINGBUFFERPO2_TEMPLATE_QUALIFIER::size_type RINGBUFFERPO2_TEMPLATE_QUALIFIER::size() const
    {
        return (_writeCount.load() - _readCount.load());
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::empty()const
    {
        return _writeCount.load() == _readCount;
    }

    
    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::full()const
    {
        return ((_writeCount - _readCount.load()) &
                (size_type)~(_mask)) != 0;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::front()
    {
        return data()[_readCount & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::front() const
    {
        return data()[_readCount & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::back()
    {
        return data()[(_writeCount-1) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::back()const
    {
        return data()[(_writeCount - 1) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::push_back(const _DataType& value)
    {
        if(full())
            return false;

        data()[_writeCount++ & _mask] = value;

        return true;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::push_back()
    {
        if(full())
            return 0;
        new(&data()[_writeCount++ & _mask]) _DataType();
        return true;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::pop_front()
    {
        if(empty())
            return false;

        ++_readCount;

        return true;
    }    

    RINGBUFFERPO2_TEMPLATE_ARGS
    void RINGBUFFERPO2_TEMPLATE_QUALIFIER::clear()
    {
        _readCount = 0;
        _writeCount = 0;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::operator[] (size_type index)
    {
        return data()[(_readCount + index) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::operator[] (size_type index)const
    {
        return data()[(_readCount + index) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType* RINGBUFFERPO2_TEMPLATE_QUALIFIER::data()
    {
        return reinterpret_cast<_DataType*>(_data);
    }
    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType* RINGBUFFERPO2_TEMPLATE_QUALIFIER::data() const
    {
        return reinterpret_cast<const _DataType*>(_data);
    }

    #define RINGBUFFER_TEMPLATE_ARGS template<unsigned _Size, typename _DataType>
    #define RINGBUFFER_TEMPLATE_QUALIFIER RingBuffer<_Size, _DataType>

    RINGBUFFER_TEMPLATE_ARGS
    RINGBUFFER_TEMPLATE_QUALIFIER::RingBuffer() : _count(0), _first(0), _last(0)
    {
    }

    RINGBUFFER_TEMPLATE_ARGS
    typename RINGBUFFER_TEMPLATE_QUALIFIER::size_type RINGBUFFER_TEMPLATE_QUALIFIER::capacity() const
    {
        return _Size;
    }

    RINGBUFFER_TEMPLATE_ARGS
    typename RINGBUFFER_TEMPLATE_QUALIFIER::size_type RINGBUFFER_TEMPLATE_QUALIFIER::size() const
    {
        return _count.load();
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::empty()const
    {
        return _count.load() == 0;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::full()const
    {
        return _count.load() == _Size;
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::front()
    {
        return data()[_first];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::front()const
    {
        return data()[_first];
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::back()
    {
        return data()[_last-1];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::back()const
    {
        return data()[_last-1];
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::push_back(const _DataType& item)
    {
        if(_count.load() == _Size)
            return false;
        data()[_last] = item;
        _last++;
        if(_last >= _Size)
            _last = 0;
        _count.fetch_add(1);
        return true;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::push_back()
    {
        if(_count.load() == _Size)
            return false;
        new (&data()[_last]) _DataType();
        ++_last;

        if(_last >= _Size)
            _last = 0;

        _count.fetch_add(1);
        
        return true;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::pop_front()
    {
        if(&_count.load() != 0)
        {
            _count.fetch_sub(1);
            ++_first;

            if(_first >= _Size)
                _first = 0;

            return true;
        }

        return false;
    }

    RINGBUFFER_TEMPLATE_ARGS
    void RINGBUFFER_TEMPLATE_QUALIFIER::clear()
    {
        do
        {
            _first = _last = 0;
        }
        while(!_count.compare_exchange_weak(_count.load(), size_type(0)));
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::operator[](size_type index)
    {
        size_type offset = _first + index;
        
        if(offset >= _Size)
            offset -= _Size;
        
        return data()[offset];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::operator[](size_type index)const
    {
        size_type offset = _first + index;
        
        if(offset >= _Size)
            offset -= _Size;
        
        return data()[offset];
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType* RINGBUFFER_TEMPLATE_QUALIFIER::data()
    {
        return reinterpret_cast<_DataType*>(_data);
    }
    RINGBUFFER_TEMPLATE_ARGS
    const _DataType* RINGBUFFER_TEMPLATE_QUALIFIER::data() const
    {
        return reinterpret_cast<const _DataType*>(_data);
    }
}

#endif //! ZHELE_RINGBUFFER_IMPL_H