/**
 * @file
 * Implement GPIO pin class
 *
 * @author Konstantin Chizhov
 * @date 2013
 * @license MIT
 */

#pragma once

#include <cstdint>

#include "traits/ioport_capabilities.h"

namespace Zhele::IO {
  /**
   * @brief Represents one GPIO pin (fully static, zero-cost abstraction)
   * 
   * @tparam _Port        Parent port type
   * @tparam _Pin         Pin number (0-based)
   * @tparam _ConfigPort  Port used for configuration (defaults to _Port)
   */
  template<typename _Port, uint8_t _Pin, typename _ConfigPort = _Port>
  class TPin : public detail::IoSpeed<_ConfigPort> {
    static_assert(_Pin < std::numeric_limits<typename _Port::DataType>::digits);

  public:
    using Port       = _Port;
    using DataType   = typename _ConfigPort::DataType;

    static constexpr unsigned Number   = _Pin;
    static constexpr bool     Inverted = false;

    using Configuration = typename _ConfigPort::Configuration;
    using DriverType    = typename _ConfigPort::DriverType;
    using PullMode      = typename _ConfigPort::PullMode;
    // Speed type alias inherited from IoSpeed<_ConfigPort> when the port supports it.
    static constexpr bool supports_speed = requires { typename _ConfigPort::Speed; };

    static void Set();
    static void Set(bool state);
    static void Clear();
    static void Toggle();
    static bool IsSet();

    static void SetDir(bool isWrite);
    static void SetDirRead();
    static void SetDirWrite();

    static void WaitForSet();
    static void WaitForClear();

    static void SetConfiguration(typename _ConfigPort::Configuration configuration);

    template<typename _ConfigPort::Configuration configuration>
    static void SetConfiguration();

    static void SetDriverType(typename _ConfigPort::DriverType driverType);

    template<typename _ConfigPort::DriverType driverType>
    static void SetDriverType();

    static void SetPullMode(typename _ConfigPort::PullMode pullMode);

    template<PullMode pullMode>
    static void SetPullMode();

    static void SetSpeed(auto speed)
    requires supports_speed;

    template<auto speed>
    static void SetSpeed()
    requires supports_speed;

    static void AltFuncNumber(uint8_t funcNumber)
    requires requires(uint8_t n) { _ConfigPort::AltFuncNumber(n); };

    template<uint8_t funcNumber>
    static void AltFuncNumber()
    requires requires { _ConfigPort::template AltFuncNumber<0, 0>(); };
  };

  /**
   * @brief Inverted pin: Set means low, Clear means high
   */
  template<typename _Port, uint8_t _Pin, typename _ConfigPort = _Port>
  class InvertedPin : public TPin<_Port, _Pin, _ConfigPort> {
  public:
    static constexpr bool Inverted = true;

    static void Set(bool val);
    static void Set();
    static void Clear();
  };
}

#include "impl/iopin.h"


