/**
 * @file
 * United header for iopins + GpioPin concept
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#pragma once

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
#include "platform/stm32/iopins.h"
#elif defined(ZHELE_PLATFORM_CH32)
#include "platform/ch32/iopins.h"
#elif defined(ZHELE_PLATFORM_NIIET)
#include "platform/niiet/iopins.h"
#else
#error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers."
#endif

#include <concepts>

namespace Zhele::IO
{
  /**
   * @brief Minimum interface every GPIO pin must satisfy
   *
   * Optional methods (SetSpeed, AltFuncNumber, …) are NOT part of this
   * concept — check T::Port::supports_* or use the sub-concepts from
   * ioports.h when you need them
   */
  template <typename T>
  concept io_pin = requires {
    typename T::Port;
    typename T::DataType;
    { T::Number   } -> std::convertible_to<unsigned>;
    { T::Inverted } -> std::convertible_to<bool>;
  } && requires(bool state) {
    { T::Set()         } -> std::same_as<void>;
    { T::Set(state)    } -> std::same_as<void>;
    { T::Clear()       } -> std::same_as<void>;
    { T::Toggle()      } -> std::same_as<void>;
    { T::IsSet()       } -> std::same_as<bool>;
    { T::SetDirRead()  } -> std::same_as<void>;
    { T::SetDirWrite() } -> std::same_as<void>;
  };

  template <typename T>
  concept speed_configurable_io_pin = io_pin<T> &&
    requires { typename T::Speed; };

} // namespace Zhele::IO


