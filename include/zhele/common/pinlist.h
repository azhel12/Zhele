/**
 * @file
 * Generic PinList ("virtual port") — platform-agnostic
 *
 * @author Aleksei Zhelonkin (based on Konstantin Chizhov)
 * @licence MIT
 */

#pragma once

#include "template_utils/type_list.h"
#include "template_utils/data_type_selector.h"
#include "traits/ioport_capabilities.h"
#include "zhele/common/template_utils/array.h"

#include <limits>
#include <type_traits>

namespace Zhele::IO {
  /**
   * @brief PinList built from an arbitrary set of pins
   */
  template<typename... _Pins>
  class PinList : public detail::IoSpeed<template_utils::type_unbox<template_utils::type_list<typename _Pins::Port...>::remove_duplicates().head()>> {
    static constexpr auto _pins  = template_utils::type_list<_Pins...>{};
    static constexpr auto _ports = template_utils::type_list<typename _Pins::Port...>::remove_duplicates();

    static_assert(!_pins.is_empty(),  "PinList cannot be empty");
    static_assert(_ports.is_unique(), "Fail to remove port duplicates!");

    using FirstPort = template_utils::type_unbox<template_utils::type_list<typename _Pins::Port...>::remove_duplicates().head()>;
  public:
    using DataType  = template_utils::type_unbox<template_utils::GetSuitableUnsignedType<_pins.size()>()>;

    using Configuration = typename FirstPort::Configuration;
    using DriverType    = typename FirstPort::DriverType;
    using PullMode      = typename FirstPort::PullMode;
    // Speed — from IoSpeed<FirstPort> when defined
    static constexpr bool supports_speed = requires { typename FirstPort::Speed; };

    static void Write(DataType value);
    
    template<DataType value>
    static void Write();

    static DataType Read();

    static void Clear(DataType value);

    template<DataType value>
    static void Clear();

    static void Set(DataType value);

    template<DataType value>
    static void Set();

    static void ClearAndSet(DataType clearMask, DataType setMask);

    template<DataType clearMask, DataType setMask>
    static void ClearAndSet();

    static void Toggle(DataType value);

    template<DataType value>
    static void Toggle();

    static DataType PinRead();

    static void Enable();

    static void Disable();
    
    static void SetConfiguration(Configuration configuration, DataType mask = std::numeric_limits<DataType>::max());

    template<Configuration configuration, DataType mask = std::numeric_limits<DataType>::max()>
    static void SetConfiguration();

    static void SetDriverType(DriverType driver, DataType mask = std::numeric_limits<DataType>::max());

    template<DriverType driver, DataType mask = std::numeric_limits<DataType>::max()>
    static void SetDriverType();

    static void SetPullMode(PullMode mode,DataType mask = std::numeric_limits<DataType>::max());

    template<PullMode mode, DataType mask = std::numeric_limits<DataType>::max()>
    static void SetPullMode();

    static void SetSpeed(auto speed, DataType mask = std::numeric_limits<DataType>::max())
    requires supports_speed;

    template<auto speed, DataType mask = std::numeric_limits<DataType>::max()>
    static void SetSpeed()
    requires supports_speed;

    static void AltFuncNumber(uint8_t number,
                              DataType mask = std::numeric_limits<DataType>::max());

    template<uint8_t number, DataType mask = std::numeric_limits<DataType>::max()>
    static void AltFuncNumber();


    template<typename Pin>
    static constexpr int IndexOf = _pins.template search<Pin>();

    template<int Index>
    using Pin = template_utils::type_unbox<_pins.template get<Index>()>;

  private:
    static constexpr auto GetPinlistValueForPort(auto port, DataType value);

    static consteval  auto GetPinlistMaskForPort(auto port);

    template<typename Port>
    static consteval auto GetPinsForPort(template_utils::type_box<Port> port);

    static DataType ExtractPinlistOutValueFromPort(auto port);

    static DataType ExtractPinlistValueFromPort(auto port);
  };

  template <ArrayU8 AltFunctions, typename... IOPins>
  struct AltPinList {
      static_assert(AltFunctions.size() == sizeof...(IOPins));
      using io_pins                          = IO::PinList<IOPins...>;
      static constexpr ArrayU8 alt_functions = AltFunctions;
  };

} // namespace Zhele::IO

#include "impl/pinlist.h"


