/**
 * @file
 * @brief Macros, templates for registers
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#ifndef ZHELE_IOREG_COMMON_H
#define ZHELE_IOREG_COMMON_H

#include <cstdint>

namespace Zhele
{

    /**
     * @brief Declare class with bit operations
     * 
     * @param REG_NAME Register name (from MCU header)
     * @param CLASS_NAME Target class name
     * @param DATA_TYPE Register data type
     * 
     * @todo Replace macro with template class by @lamer0k
     * see https://github.com/lamer0k/CortexLib/blob/master/AbstractHardware/Registers/register.hpp
     */
    #define IO_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
    class CLASS_NAME\
    {\
    public:\
        using DataT = DATA_TYPE;\
        static DataT Get(){return REG_NAME;}\
        static void Set(DataT value){REG_NAME = value;}\
        static void Or(DataT value){REG_NAME |= value;}\
        static void And(DataT value){REG_NAME &= value;}\
        static void Xor(DataT value){REG_NAME ^= value;}\
        static void AndOr(DataT andMask, DataT orMask){REG_NAME = (REG_NAME & andMask) | orMask;}\
        template<unsigned Bit>\
        static bool IsBitSet(){return REG_NAME & (1 << Bit);}\
        template<unsigned Bit>\
        static bool IsBitClear(){return !(REG_NAME & (1 << Bit));}\
    }

    template<uint32_t _Address, typename _DataType>
    class RegisterWrapper
    {
    public:
        static _DataType Get(){return  *reinterpret_cast<_DataType*>(_Address);}
        static void Set(_DataType value){ *reinterpret_cast<_DataType*>(_Address) = value;}
        static void Or(_DataType value){ *reinterpret_cast<_DataType*>(_Address) |= value;}
        static void And(_DataType value){ *reinterpret_cast<_DataType*>(_Address) &= value;}
        static void Xor(_DataType value){ *reinterpret_cast<_DataType*>(_Address) ^= value;}
        static void AndOr(_DataType andMask, _DataType orMask){ *reinterpret_cast<_DataType*>(_Address) = ( *reinterpret_cast<_DataType*>(_Address) & andMask) | orMask;}
        template<unsigned Bit>
        static bool IsBitSet(){return  *reinterpret_cast<_DataType*>(_Address) & (1 << Bit);}
        template<unsigned Bit>
        static bool IsBitClear(){return !( *reinterpret_cast<_DataType*>(_Address) & (1 << Bit));}
    };

    /**
     * @brief Wrap around some type
     */
    #define I_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
    class CLASS_NAME\
    {\
    public:\
        using DataT = DATA_TYPE;\
        static DataT Get(){return REG_NAME;}\
    }

    #define IO_STRUCT_WRAPPER(STRUCT_PTR, CLASS_NAME, STRUCT_TYPE) \
    class CLASS_NAME\
    {\
    public:\
        using DataT = STRUCT_TYPE;\
        static DataT* Get(){return ((DataT*)STRUCT_PTR);}\
        DataT* operator->(){return ((DataT*)(STRUCT_PTR));}\
    }

    /**
     * @brief Bitfield wrapper
     */
    #define IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE, BITFIELD_OFFSET, BITFIELD_LENGTH) \
    class CLASS_NAME\
    {\
    public:\
        using DataT = DATA_TYPE;\
        using RegT = decltype(REG_NAME);\
        static constexpr RegT Mask = ((RegT(1u) << BITFIELD_LENGTH) - 1);\
        static DataT Get(){return static_cast<DataT>((REG_NAME >> BITFIELD_OFFSET) & Mask);}\
        static void Set(DataT value){REG_NAME = (REG_NAME & ~(Mask << BITFIELD_OFFSET)) | (((RegT)value & Mask) << BITFIELD_OFFSET);}\
    }
    
    #define DECLARE_IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, CMSIS_DEFINE) \
        const static unsigned CLASS_NAME##Offset = CMSIS_DEFINE##_Pos; \
        const static unsigned CLASS_NAME##Length = GetBitFieldLength<(CMSIS_DEFINE##_Msk >> CMSIS_DEFINE##_Pos)>; \
        IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, uint32_t, CLASS_NAME##Offset, CLASS_NAME##Length); \

    
    /**
     * @brief Dummy register
     * 
     * @details
     * Any data written is ignored
     * Any reads returns zero
     * 
     * @tparam _DataType Register data type
     */
    template<typename _DataType = unsigned char>
    class NullReg
    {
    public:
        using DataT = _DataType;
        static DataT Get(){return DataT(0);}
        static void Set(DataT){}
        static void Or(DataT){}
        static void And(DataT){}
        static void Xor(DataT){}
        static void AndOr(DataT, DataT){}
        template<unsigned Bit>
        static bool IsBitSet(){return false;}
        template<unsigned Bit>
        static bool IsBitClear(){return true;}
    };
    
    /**
     * @brief
     * Implements I/O register
     * 
     * @tparam _RegAddr Register address (from MCU main CMSIS header)
     * @tparam _DataType Register data type
     */
    template<unsigned _RegAddr, typename _DataType = unsigned char>
    class IoReg
    {
    public:
        using DataT = _DataType;
        
        static volatile DataT& Value(){ return *static_cast<DataT*>(_RegAddr);}
        static DataT Get(){return Value();}
        static void Set(DataT value){Value() = value;}
        static void Or(DataT value){Value() |= value;}
        static void And(DataT value){Value() &= value;}
        static void Xor(DataT value){Value() ^= value;}
        static void AndOr(DataT andMask, DataT orMask){Value() = (Value() & andMask) | orMask;}
        template<int Bit>
        static bool BitIsSet(){return Value() & (1 << Bit);}
        template<int Bit>
        static bool BitIsClear(){return !(Value() & (1 << Bit));}
    };
    

    /**
     * @brief I\O register wrapper.
     * 
     * @tparam _RegAddr Register address
     * @tparam _WrapperType I/O register wrapper class type
     */
    template<unsigned _RegAddr, typename _WrapperType>
    class IoStruct
    {
    public:
        using DataT = _WrapperType;
        static DataT* Get(){return static_cast<DataT*>(_RegAddr);}
        DataT* operator->(){return Get();}
    };
    
    /**
     * @brief I/O register bitfield wrapper
     * 
     * @tparam _RegAddr Register address
     * @tparam _DataType Register data type
     * @tparam _BitfieldOffset Bitfield offset in register
     * @tparam _BitfieldLength Bitfield length
     */
    template<unsigned _RegAddr, typename _DataType, unsigned _BitfieldOffset, unsigned _BitfieldLength>
    class IoBitfield
    {
    public:
        using DataT = _DataType;
        static volatile DataT& Value(){ return *static_cast<DataT*>(_RegAddr);}
        static constexpr DataT Mask = ((DataT(1u) << _BitfieldLength) - 1);
        static DataT Get(){return (Value() >> _BitfieldOffset) & Mask;}
        static void Set(DataT value){Value() = (Value() & ~(Mask << _BitfieldOffset)) | ((value & Mask) << _BitfieldOffset);}
    };

    /**
     * @brief I/O register bit wrapper
     * 
     * @tparam _RegAddr Register address
     * @tparam _DataType Register data type
     * @tparam _BitfieldOffset Bitfield offset in register
     */
    template<unsigned _RegAddr, typename _DataType, unsigned _BitfieldOffset>
    class IoBit
    {
    public:
        static volatile _DataType& Value(){ return *reinterpret_cast<_DataType*>(_RegAddr);}
        static bool IsSet(){ return ((Value() >> _BitfieldOffset) & 0x01) != 0; }
        static void Set(){ Value() |= 1 << _BitfieldOffset; }
        static void Clear(){ Value() &= ~(1 << _BitfieldOffset); }
    };

    /**
     * @brief Calculate bitfield length (if bitfield is continuous) in compile-time
     * 
     * @tparam _Mask Bitfield mask
     */
    template<uint32_t _Mask>
    class BitFieldLength
    {
    public:
        static_assert((_Mask & 1) == 1);
        static const uint32_t value = 1 + BitFieldLength<(_Mask >> 1)>::value;
    };
    template<>
    class BitFieldLength<0>
    {
    public:
        static const uint32_t value = 0;
    };
    template<uint32_t _Mask>
    inline constexpr uint32_t GetBitFieldLength = BitFieldLength<_Mask>::value;
}
#endif //! ZHELE_IOREG_COMMON_H
