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

namespace Zhele::Containers
{
    #define RINGBUFFERPO2_TEMPLATE_ARGS template<unsigned _Size, typename _DataType, typename _Atomic>
    #define RINGBUFFERPO2_TEMPLATE_QUALIFIER RingBufferPO2<_Size, _DataType, _Atomic>

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
    typename RINGBUFFERPO2_TEMPLATE_QUALIFIER::size_type RingBufferPO2<_Size,_DataType, _Atomic>::size() const
    {
        return (_Atomic::Fetch(&_writeCount) - _Atomic::Fetch(&_readCount));
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::empty()const
    {
        return _Atomic::Fetch(&_writeCount) == _readCount;
    }

    
    RINGBUFFERPO2_TEMPLATE_ARGS
    bool RINGBUFFERPO2_TEMPLATE_QUALIFIER::full()const
    {
        return ((_writeCount - _Atomic::Fetch(&_readCount)) &
                (size_type)~(_mask)) != 0;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::front()
    {
        //MCUCPP_ASSERT(!empty());
        return data()[_readCount & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::front() const
    {
        //MCUCPP_ASSERT(!empty());
        return data()[_readCount & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::back()
    {
        //MCUCPP_ASSERT(!empty());
        return data()[(_writeCount-1) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::back()const
    {
        //MCUCPP_ASSERT(!empty());
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
        _readCount=0;
        _writeCount=0;
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::operator[] (size_type index)
    {
        //MCUCPP_ASSERT(index < _Size);
        return data()[(_readCount + index) & _mask];
    }

    RINGBUFFERPO2_TEMPLATE_ARGS
    const _DataType& RINGBUFFERPO2_TEMPLATE_QUALIFIER::operator[] (size_type index)const
    {
        //MCUCPP_ASSERT(index < _Size);
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

    #define RINGBUFFER_TEMPLATE_ARGS template<unsigned _Size, typename _DataType, typename _Atomic>
    #define RINGBUFFER_TEMPLATE_QUALIFIER RingBuffer<_Size, _DataType, _Atomic>

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
        return _Atomic::Fetch(&_count);
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::empty()const
    {
        return _Atomic::Fetch(&_count) == 0;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::full()const
    {
        return _Atomic::Fetch(&_count) == _Size;
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::front()
    {
        //MCUCPP_ASSERT(!empty());
        return data()[_first];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::front()const
    {
        //MCUCPP_ASSERT(!empty());
        return data()[_first];
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::back()
    {
        MCUCPP_ASSERT(!empty());
        return data()[_last-1];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::back()const
    {
        MCUCPP_ASSERT(!empty());
        return data()[_last-1];
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::push_back(const _DataType& item)
    {
        if(_Atomic::Fetch(&_count) == _Size)
            return false;
        data()[_last] = item;
        _last++;
        if(_last >= _Size)
            _last = 0;
        _Atomic::FetchAndAdd(&_count, 1);
        return true;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::push_back()
    {
        if(_Atomic::Fetch(&_count) == _Size)
            return false;
        new (&data()[_last]) _DataType();
        ++_last;

        if(_last >= _Size)
            _last = 0;

        _Atomic::FetchAndAdd(&_count, 1);
        
        return true;
    }

    RINGBUFFER_TEMPLATE_ARGS
    bool RINGBUFFER_TEMPLATE_QUALIFIER::pop_front()
    {
        if(_Atomic::Fetch(&_count) != 0)
        {
            _Atomic::FetchAndSub(&_count, 1);
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
        while(!_Atomic::CompareExchange(&_count, _count, size_type(0)));
    }

    RINGBUFFER_TEMPLATE_ARGS
    _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::operator[](size_type index)
    {
        //MCUCPP_ASSERT(index < _Size);
        size_type offset = _first + index;
        
        if(offset >= _Size)
            offset -= _Size;
        
        return data()[offset];
    }

    RINGBUFFER_TEMPLATE_ARGS
    const _DataType& RINGBUFFER_TEMPLATE_QUALIFIER::operator[](size_type index)const
    {
        //MCUCPP_ASSERT(index < _Size);
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