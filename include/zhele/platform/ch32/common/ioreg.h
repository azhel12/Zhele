/**
 * @file
 * Macros and templates for hardware register access (CH32).
 *
 * @author Konstantin Chizhov
 * @date 2018
 * @license MIT
 */
#pragma once

#include <cstdint>

namespace Zhele
{

  /**
   * @brief Wrap a plain hardware register (e.g. a global like RCC->CR) as a class.
   *
   * @param REG_NAME  Register expression (must be addressable as an lvalue)
   * @param CLASS_NAME  Name of the generated class
   * @param DATA_TYPE  Register width type (uint8_t / uint16_t / uint32_t)
   */
  #define IO_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
  class CLASS_NAME \
  { \
  public: \
    using DataT = DATA_TYPE; \
    static DataT Get()                        { return REG_NAME; } \
    static void Set(DataT value)              { REG_NAME = value; } \
    static void Or(DataT value)               { REG_NAME |= value; } \
    static void And(DataT value)              { REG_NAME &= value; } \
    static void Xor(DataT value)              { REG_NAME ^= value; } \
    static void AndOr(DataT andM, DataT orM)  { REG_NAME = (REG_NAME & andM) | orM; } \
    template<unsigned Bit> \
    static bool IsBitSet()   { return (REG_NAME & (1u << Bit)) != 0; } \
    template<unsigned Bit> \
    static bool IsBitClear() { return (REG_NAME & (1u << Bit)) == 0; } \
  }

  /**
   * @brief Read-only wrapper around a plain register.
   */
  #define I_REG_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE) \
  class CLASS_NAME \
  { \
  public: \
    using DataT = DATA_TYPE; \
    static DataT Get() { return REG_NAME; } \
  }

  /**
   * @brief Wrap a peripheral struct pointer (e.g. GPIOA -> GPIO_TypeDef*) as a callable class.
   *
   * Usage: IO_STRUCT_WRAPPER(GPIOA, GpioaRegs, GPIO_TypeDef);
   * Then call GpioaRegs() to get GPIO_TypeDef*.
   */
  #define IO_STRUCT_WRAPPER(STRUCT_PTR, CLASS_NAME, STRUCT_TYPE) \
  class CLASS_NAME \
  { \
  public: \
    using DataT = STRUCT_TYPE; \
    static DataT* Get()       { return reinterpret_cast<DataT*>(STRUCT_PTR); } \
    DataT* operator->() const { return reinterpret_cast<DataT*>(STRUCT_PTR); } \
    DataT* operator()() const { return reinterpret_cast<DataT*>(STRUCT_PTR); } \
  }

  /**
   * @brief Wrap a bitfield inside a register.
   */
  #define IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, DATA_TYPE, BITFIELD_OFFSET, BITFIELD_LENGTH) \
  class CLASS_NAME \
  { \
  public: \
    using DataT = DATA_TYPE; \
    using RegT  = decltype(REG_NAME); \
    static constexpr RegT  Mask     = ((RegT(1u) << BITFIELD_LENGTH) - 1); \
    static constexpr unsigned MaxValue = (1u << BITFIELD_LENGTH) - 1; \
    static DataT Get() { return static_cast<DataT>((REG_NAME >> BITFIELD_OFFSET) & Mask); } \
    static void Set(DataT value) { REG_NAME = (REG_NAME & ~(Mask << BITFIELD_OFFSET)) | ((static_cast<RegT>(value) & Mask) << BITFIELD_OFFSET); } \
  }

  #define DECLARE_IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, CMSIS_DEFINE) \
    static constexpr unsigned CLASS_NAME##Offset = CMSIS_DEFINE##_Pos; \
    static constexpr unsigned CLASS_NAME##Length = GetBitFieldLength<(CMSIS_DEFINE##_Msk >> CMSIS_DEFINE##_Pos)>; \
    IO_BITFIELD_WRAPPER(REG_NAME, CLASS_NAME, uint32_t, CLASS_NAME##Offset, CLASS_NAME##Length);

  // -------------------------------------------------------------------------
  // Template register wrappers
  // -------------------------------------------------------------------------

  /**
   * @brief Typed wrapper around a fixed register address.
   */
  template<uint32_t _Address, typename _DataType>
  class RegisterWrapper
  {
  public:
    using DataT = _DataType;

    static volatile DataT& Value() { return *reinterpret_cast<volatile DataT*>(_Address); }
    static DataT Get()                          { return Value(); }
    static void Set(DataT v)                    { Value() = v; }
    static void Or(DataT v)                     { Value() |= v; }
    static void And(DataT v)                    { Value() &= v; }
    static void Xor(DataT v)                    { Value() ^= v; }
    static void AndOr(DataT andM, DataT orM)    { Value() = (Value() & andM) | orM; }
    template<unsigned Bit>
    static bool IsBitSet()   { return (Value() & (DataT(1u) << Bit)) != 0; }
    template<unsigned Bit>
    static bool IsBitClear() { return (Value() & (DataT(1u) << Bit)) == 0; }
  };

  /**
   * @brief Dummy register — writes are discarded, reads return zero.
   */
  template<typename _DataType = uint8_t>
  class NullReg
  {
  public:
    using DataT = _DataType;
    static DataT Get()                       { return DataT(0); }
    static void Set(DataT)                   {}
    static void Or(DataT)                    {}
    static void And(DataT)                   {}
    static void Xor(DataT)                   {}
    static void AndOr(DataT, DataT)          {}
    template<unsigned Bit> static bool IsBitSet()   { return false; }
    template<unsigned Bit> static bool IsBitClear() { return true; }
  };

  /**
   * @brief Register at a compile-time address.
   */
  template<unsigned _RegAddr, typename _DataType = uint8_t>
  class IoReg
  {
  public:
    using DataT = _DataType;

    static volatile DataT& Value() { return *reinterpret_cast<volatile DataT*>(_RegAddr); }
    static DataT Get()                       { return Value(); }
    static void Set(DataT v)                 { Value() = v; }
    static void Or(DataT v)                  { Value() |= v; }
    static void And(DataT v)                 { Value() &= v; }
    static void Xor(DataT v)                 { Value() ^= v; }
    static void AndOr(DataT andM, DataT orM) { Value() = (Value() & andM) | orM; }
    template<int Bit> static bool BitIsSet()   { return (Value() & (DataT(1u) << Bit)) != 0; }
    template<int Bit> static bool BitIsClear() { return (Value() & (DataT(1u) << Bit)) == 0; }
  };

  /**
   * @brief Peripheral struct at a compile-time address.
   */
  template<unsigned _RegAddr, typename _WrapperType>
  class IoStruct
  {
  public:
    using DataT = _WrapperType;
    static DataT* Get()       { return reinterpret_cast<DataT*>(_RegAddr); }
    DataT* operator->() const { return reinterpret_cast<DataT*>(_RegAddr); }
  };

  /**
   * @brief Bitfield at a compile-time address.
   */
  template<unsigned _RegAddr, typename _DataType, unsigned _BitfieldOffset, unsigned _BitfieldLength>
  class IoBitfield
  {
  public:
    using DataT = _DataType;

    static volatile DataT& Value() { return *reinterpret_cast<volatile DataT*>(_RegAddr); }
    static constexpr DataT Mask = ((DataT(1u) << _BitfieldLength) - 1);
    static DataT Get() { return (Value() >> _BitfieldOffset) & Mask; }
    static void Set(DataT value)
    {
      Value() = (Value() & ~static_cast<DataT>(Mask << _BitfieldOffset))
              | static_cast<DataT>((value & Mask) << _BitfieldOffset);
    }
  };

  /**
   * @brief Single bit at a compile-time address.
   */
  template<unsigned _RegAddr, typename _DataType, unsigned _BitfieldOffset>
  class IoBit
  {
  public:
    static volatile _DataType& Value() { return *reinterpret_cast<volatile _DataType*>(_RegAddr); }
    static bool IsSet()  { return ((Value() >> _BitfieldOffset) & 0x01u) != 0; }
    static void Set()    { Value() |=  (_DataType(1u) << _BitfieldOffset); }
    static void Clear()  { Value() &= ~(_DataType(1u) << _BitfieldOffset); }
  };

  // -------------------------------------------------------------------------
  // Compile-time bitfield length utility
  // -------------------------------------------------------------------------

  template<uint32_t _Mask>
  class BitFieldLength
  {
  public:
    static_assert((_Mask & 1u) == 1u, "Mask must start at bit 0");
    static constexpr uint32_t value = 1u + BitFieldLength<(_Mask >> 1)>::value;
  };
  template<>
  class BitFieldLength<0>
  {
  public:
    static constexpr uint32_t value = 0;
  };

  template<uint32_t _Mask>
  inline constexpr uint32_t GetBitFieldLength = BitFieldLength<_Mask>::value;

} // namespace Zhele


