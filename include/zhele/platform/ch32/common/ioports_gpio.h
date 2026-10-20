/**
 * @file
 * CH32 GPIO: NativePortBase, NullPort, PortImplementation (CFGLR/CFGHR, INDR/OUTDR, BSHR/BCR).
 * Shared by all series where GPIO_TypeDef matches WCH layout. Include after device header.
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IOPORTS_GPIO_H
#define ZHELE_PLATFORM_CH32_COMMON_IOPORTS_GPIO_H

#include "../platform_detector.h"

#include <cstdint>
#include <limits>

namespace Zhele
{
  namespace IO
  {
    class NativePortBase
    {
    public:
#if defined(ZHELE_CH32_FAMILY_X0)
      using DataType = uint32_t;
#else
      using DataType = uint16_t;
#endif

      // The new V00x GPIO (V002/004/005/006/007) narrows MODE to a single bit
      enum Configuration
      {
        Analog  = 0,
        In      = 0x04,
#if defined(CH32V00X) || defined(ZHELE_CH32_FAMILY_X0)
        Out     = 0x01,
        AltFunc = 0x09
#else
        Out     = 0x03,
        AltFunc = 0x0B
#endif
      };

      enum PullMode
      {
        NoPull   = 0,
        PullUp   = 0x08,
        PullDown = 0x18
      };

#if defined(ZHELE_CH32_FAMILY_X0)
      enum DriverType
      {
        PushPull  = 0,
        OpenDrain = 0
      };

      enum Speed
      {
        Slow   = 1,
        Medium = 1,
        Fast   = 1
      };
#else
      enum DriverType
      {
        PushPull  = 0,
        OpenDrain = 4
      };

      enum Speed
      {
        Slow   = 2,
        Medium = 1,
        Fast   = 3
      };
#endif

      static void Write(DataType value);
      template <DataType value>
      static void Write();

      static DataType Read();

      static void Clear(DataType value);
      template <DataType value>
      static void Clear();

      static void Set(DataType value);
      template <DataType value>
      static void Set();

      static void ClearAndSet(DataType clearMask, DataType setMask);
      template <DataType clearMask, DataType setMask>
      static void ClearAndSet();

      static void Toggle(DataType value);
      template <DataType value>
      static void Toggle();

      static DataType PinRead();

      static void SetConfiguration(Configuration configuration, DataType mask = std::numeric_limits<DataType>::max());
      template <Configuration configuration, DataType mask = std::numeric_limits<DataType>::max()>
      static void SetConfiguration();

      static void SetDriverType(DriverType driver, DataType mask = std::numeric_limits<DataType>::max());
      template <DriverType driver, DataType mask = std::numeric_limits<DataType>::max()>
      static void SetDriverType();

      static void SetPullMode(PullMode mode, DataType mask = std::numeric_limits<DataType>::max());
      template <PullMode mode, DataType mask = std::numeric_limits<DataType>::max()>
      static void SetPullMode();

      static void SetSpeed(Speed speed, DataType mask = std::numeric_limits<DataType>::max());
      template <Speed speed, DataType mask = std::numeric_limits<DataType>::max()>
      static void SetSpeed();

      static void AltFuncNumber(uint8_t number, DataType mask = std::numeric_limits<DataType>::max());
      template <uint8_t number, DataType mask = std::numeric_limits<DataType>::max()>
      static void AltFuncNumber();

      static void Enable();
      static void Disable();
    };

    class NullPort : public NativePortBase
    {
    public:
      static void Write(DataType) {}
      template <DataType>
      static void Write() {}

      static DataType Read() { return DataType{}; }

      static void Clear(DataType) {}
      template <DataType>
      static void Clear() {}

      static void Set(DataType) {}
      template <DataType>
      static void Set() {}

      static void ClearAndSet(DataType, DataType) {}
      template <DataType, DataType>
      static void ClearAndSet() {}

      static void Toggle(DataType) {}
      template <DataType>
      static void Toggle() {}

      static DataType PinRead() { return DataType{}; }

      static void SetConfiguration(Configuration, DataType = std::numeric_limits<DataType>::max()) {}
      template <Configuration, DataType = std::numeric_limits<DataType>::max()>
      static void SetConfiguration() {}

      static void SetDriverType(DriverType, DataType = std::numeric_limits<DataType>::max()) {}
      template <DriverType, DataType = std::numeric_limits<DataType>::max()>
      static void SetDriverType() {}

      static void SetPullMode(PullMode, DataType = std::numeric_limits<DataType>::max()) {}
      template <PullMode, DataType = std::numeric_limits<DataType>::max()>
      static void SetPullMode() {}

      static void SetSpeed(Speed, DataType = std::numeric_limits<DataType>::max()) {}
      template <Speed, DataType = std::numeric_limits<DataType>::max()>
      static void SetSpeed() {}

      static void AltFuncNumber(uint8_t, DataType = std::numeric_limits<DataType>::max()) {}
      template <uint8_t, DataType = std::numeric_limits<DataType>::max()>
      static void AltFuncNumber() {}

      static void Enable() {}
      static void Disable() {}

      enum { Id = '-' };
    };

    namespace Private
    {
      template <typename _Regs, typename _ClkEnReg, uint8_t ID>
      class PortImplementation : public NativePortBase
      {
      public:
        static DataType Read()
        {
          return static_cast<DataType>(_Regs()->OUTDR);
        }

        static void Write(DataType value)
        {
          _Regs()->OUTDR = value;
        }

        // CH32 has no single-write atomic clear+set (unlike STM32 BSRR).
        // Clear before set so that if a pin appears in both masks it ends up set,
        // matching STM32 BSRR semantics.
        static void ClearAndSet(DataType clearMask, DataType setMask)
        {
          Clear(clearMask);
          Set(setMask);
        }

        static void Set(DataType value)
        {
          _Regs()->BSHR = value & 0xffffu;
#if defined(ZHELE_CH32_FAMILY_X0)
          _Regs()->BSXR = value >> 16;
#endif
        }

        static void Clear(DataType value)
        {
          _Regs()->BCR = value;
        }

        static void Toggle(DataType value)
        {
          _Regs()->OUTDR ^= value;
        }

        static DataType PinRead()
        {
          return static_cast<DataType>(_Regs()->INDR);
        }

        template <DataType clearMask, DataType setMask>
        static void ClearAndSet()
        {
          Clear<clearMask>();
          Set<setMask>();
        }

        template <DataType value>
        static void Toggle()
        {
          _Regs()->OUTDR ^= value;
        }

        template <DataType value>
        static void Set()
        {
          if constexpr ((value & 0xffffu) != 0)
            _Regs()->BSHR = value & 0xffffu;
#if defined(ZHELE_CH32_FAMILY_X0)
          if constexpr ((value >> 16) != 0)
            _Regs()->BSXR = value >> 16;
#endif
        }

        template <DataType value>
        static void Clear()
        {
          _Regs()->BCR = value;
        }

        static void SetConfiguration(Configuration configuration, DataType mask = std::numeric_limits<DataType>::max())
        {
          ModifyConfiguration(mask, 0x0f, configuration);
        }

        template <Configuration configuration, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetConfiguration()
        {
          ModifyConfiguration<mask, 0x0f>(configuration);
        }

        static void SetSpeed(Speed speed, DataType mask = std::numeric_limits<DataType>::max())
        {
          ModifyConfiguration(mask, SpeedMask, speed);
        }

        template <Speed speed, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetSpeed()
        {
          ModifyConfiguration<mask, SpeedMask>(speed);
        }

        static void SetPullMode(PullMode mode, DataType mask = std::numeric_limits<DataType>::max())
        {
          ModifyConfiguration(mask, 0x0f, mode & 0x08u);

          if (mode & 0x10u)
            Clear(mask);
          else
            Set(mask);
        }

        template <PullMode mode, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetPullMode()
        {
          ModifyConfiguration<mask, 0x0f>(mode & 0x08u);

          if constexpr (mode & 0x10u)
            Clear<mask>();
          else
            Set<mask>();
        }

        static void SetDriverType(DriverType driver, DataType mask = std::numeric_limits<DataType>::max())
        {
          ModifyConfiguration(mask, 0x04, driver);
        }

        template <DriverType driver, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetDriverType()
        {
          ModifyConfiguration<mask, 0x04>(driver);
        }

        // CH32V0 remaps peripherals globally via AFIO->PCFR, not per-pin.
        static void AltFuncNumber(uint8_t, DataType = std::numeric_limits<DataType>::max()) {}
        template <uint8_t, DataType = std::numeric_limits<DataType>::max()>
        static void AltFuncNumber() {}

        static void Enable()  { _ClkEnReg::Enable(); }
        static void Disable() { _ClkEnReg::Disable(); }

        enum { Id = ID };

      private:
#if defined(ZHELE_CH32_FAMILY_X0)
        static constexpr unsigned SpeedMask = 0x01;
#else
        static constexpr unsigned SpeedMask = 0x03;
#endif

        static void ModifyConfiguration(DataType mask, unsigned configMask, unsigned value)
        {
          _Regs()->CFGLR = UnpackConfig(mask, _Regs()->CFGLR, value, configMask);
#if !defined(ZHELE_CH32_FAMILY_V0)
          _Regs()->CFGHR = UnpackConfig(mask >> 8, _Regs()->CFGHR, value, configMask);
#endif
#if defined(ZHELE_CH32_FAMILY_X0)
          _Regs()->CFGXR = UnpackConfig(mask >> 16, _Regs()->CFGXR, value, configMask);
#endif
        }

        template <DataType mask, unsigned configMask>
        static void ModifyConfiguration(unsigned value)
        {
          constexpr unsigned lowMaskPart = ConfigurationMask(mask);
          if constexpr (lowMaskPart != 0)
            _Regs()->CFGLR = (_Regs()->CFGLR & ~(lowMaskPart * configMask)) | lowMaskPart * value;
#if !defined(ZHELE_CH32_FAMILY_V0)
          constexpr unsigned highMaskPart = ConfigurationMask(mask >> 8);
          if constexpr (highMaskPart != 0)
            _Regs()->CFGHR = (_Regs()->CFGHR & ~(highMaskPart * configMask)) | highMaskPart * value;
#endif
#if defined(ZHELE_CH32_FAMILY_X0)
          constexpr unsigned extMaskPart = ConfigurationMask(mask >> 16);
          if constexpr (extMaskPart != 0)
            _Regs()->CFGXR = (_Regs()->CFGXR & ~(extMaskPart * configMask)) | extMaskPart * value;
#endif
        }

        // Expand an 8-bit pin mask to a 32-bit mask with one bit per 4-bit config field.
        static constexpr unsigned ConfigurationMask(unsigned mask)
        {
          mask &= 0xffu;
          const unsigned mask1 = (mask & 0xf0u) << 12 | (mask & 0x0fu);
          const unsigned mask2 = (mask1 & 0x000C000Cu) << 6 | (mask1 & 0x00030003u);
          return (mask2 & 0x02020202u) << 3 | (mask2 & 0x01010101u);
        }

        static constexpr unsigned UnpackConfig(unsigned mask, unsigned value, unsigned configuration, unsigned configMask)
        {
          mask = ConfigurationMask(mask);
          return (value & ~(mask * configMask)) | mask * configuration;
        }
      };
    }
  }
}

#endif // ZHELE_PLATFORM_CH32_COMMON_IOPORTS_GPIO_H
