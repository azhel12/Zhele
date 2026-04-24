/**
 * @file
 * United header for ioports + io_port concept
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#pragma once

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
  #include "platform/stm32/ioports.h"
#elif defined(ZHELE_PLATFORM_CH32)
  #include "platform/ch32/ioports.h"
#elif defined(ZHELE_PLATFORM_NIIET)
  #include "platform/niiet/ioports.h"
#else
  #error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers"
#endif

#include <concepts>

namespace Zhele::IO {
  /**
    * @brief Common interface for ioport
    */
  template <typename T>
  concept io_port =
    requires {
      typename T::DataType;
    } &&
    requires(typename T::DataType mask) {
      { T::Enable()     } -> std::same_as<void>;
      { T::Disable()    } -> std::same_as<void>;
      { T::Read()       } -> std::same_as<typename T::DataType>;
      { T::PinRead()    } -> std::same_as<typename T::DataType>;
      { T::Write(mask)  } -> std::same_as<void>;
      { T::Set(mask)    } -> std::same_as<void>;
      { T::Clear(mask)  } -> std::same_as<void>;
      { T::Toggle(mask) } -> std::same_as<void>;
    };

  template <typename T>
  concept mode_configurable_io_port = io_port<T> &&
    requires { typename T::Configuration; };

  template <typename T>
  concept driver_type_io_port = io_port<T> &&
    requires { typename T::DriverType; };

  template <typename T>
  concept pull_mode_io_port = io_port<T> &&
    requires { typename T::PullMode; };

  template <typename T>
  concept speed_io_port = io_port<T> &&
    requires { typename T::Speed; };

  template <typename T>
  concept alt_func_io_port = io_port<T> &&
    requires(uint8_t n) { T::AltFuncNumber(n); };

} // namespace Zhele::IO


