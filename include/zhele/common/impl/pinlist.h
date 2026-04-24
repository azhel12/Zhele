/**
 * @file
 * Generic PinList methods implementation — platform-agnostic
 *
 * @author Aleksei Zhelonkin (based on Konstantin Chizhov)
 * @licence MIT
 */
#pragma once

namespace Zhele::IO {
  template<typename... _Pins>
  void PinList<_Pins...>::Write(typename PinList<_Pins...>::DataType value) {
    _ports.foreach([value](auto port) {
      if constexpr (GetPinsForPort(port).size() == std::numeric_limits<typename decltype(port)::DataType>::digits)
        port.Write(GetPinlistValueForPort(port, value));
      else
        port.ClearAndSet(GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DataType value>
  void PinList<_Pins...>::Write() {
    _ports.foreach([](auto port) {
      if constexpr (GetPinsForPort(port).size() == std::numeric_limits<typename decltype(port)::DataType>::digits)
        port.template Write<GetPinlistValueForPort(port, value)>();
      else
        port.template ClearAndSet<GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value)>();
    });
  }

  template<typename... _Pins>
  typename PinList<_Pins...>::DataType PinList<_Pins...>::Read() {
    auto result = DataType{};
    _ports.foreach([&result](auto port) {
      result |= ExtractPinlistOutValueFromPort(port);
    });
    return result;
  }

  template<typename... _Pins>
  void PinList<_Pins...>::Clear(typename PinList<_Pins...>::DataType value) {
    _ports.foreach([value](auto port) {
      port.Clear(GetPinlistValueForPort(port, value));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DataType value>
  void PinList<_Pins...>::Clear() {
    _ports.foreach([](auto port) {
      port.template Clear<GetPinlistValueForPort(port, value)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::Set(typename PinList<_Pins...>::DataType value) {
    _ports.foreach([value](auto port) {
      port.Set(GetPinlistValueForPort(port, value));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DataType value>
  void PinList<_Pins...>::Set() {
    _ports.foreach([](auto port) {
      port.template Set<GetPinlistValueForPort(port, value)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::ClearAndSet(typename PinList<_Pins...>::DataType clearMask, typename PinList<_Pins...>::DataType setMask) {
    _ports.foreach([clearMask, setMask](auto port) {
      port.ClearAndSet(GetPinlistValueForPort(port, clearMask), GetPinlistValueForPort(port, setMask));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DataType clearMask, typename PinList<_Pins...>::DataType setMask>
  void PinList<_Pins...>::ClearAndSet() {
    _ports.foreach([](auto port) {
      port.template ClearAndSet<GetPinlistValueForPort(port, clearMask), GetPinlistValueForPort(port, setMask)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::Toggle(typename PinList<_Pins...>::DataType value) {
    _ports.foreach([value](auto port) {
      port.Toggle(GetPinlistValueForPort(port, value));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DataType value>
  void PinList<_Pins...>::Toggle() {
    _ports.foreach([](auto port) {
      port.template Toggle<GetPinlistValueForPort(port, value)>();
    });
  }

  template<typename... _Pins>
  typename PinList<_Pins...>::DataType PinList<_Pins...>::PinRead() {
    auto result = DataType{};
    _ports.foreach([&result](auto port) {
      result |= ExtractPinlistValueFromPort(port);
    });
    return result;
  }

  template<typename... _Pins>
  void PinList<_Pins...>::Enable() {
    _ports.foreach([](auto port) { port.Enable(); });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::Disable() {
    _ports.foreach([](auto port) { port.Disable(); });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::SetConfiguration(typename PinList<_Pins...>::Configuration config, typename PinList<_Pins...>::DataType mask) {
    _ports.foreach([mask, config](auto port) {
      port.SetConfiguration(config, GetPinlistValueForPort(port, mask));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::Configuration config, typename PinList<_Pins...>::DataType mask>
  void PinList<_Pins...>::SetConfiguration() {
    _ports.foreach([](auto port) {
      port.template SetConfiguration<config, GetPinlistValueForPort(port, mask)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::SetDriverType(typename PinList<_Pins...>::DriverType driver, typename PinList<_Pins...>::DataType mask) {
    _ports.foreach([mask, driver](auto port) {
      port.SetDriverType(driver, GetPinlistValueForPort(port, mask));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::DriverType driver, typename PinList<_Pins...>::DataType mask>
  void PinList<_Pins...>::SetDriverType() {
    _ports.foreach([](auto port) {
      port.template SetDriverType<driver, GetPinlistValueForPort(port, mask)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::SetPullMode(typename PinList<_Pins...>::PullMode mode, typename PinList<_Pins...>::DataType mask) {
    _ports.foreach([mask, mode](auto port) {
      port.SetPullMode(mode, GetPinlistValueForPort(port, mask));
    });
  }

  template<typename... _Pins>
  template<typename PinList<_Pins...>::PullMode mode, typename PinList<_Pins...>::DataType mask>
  void PinList<_Pins...>::SetPullMode() {
    _ports.foreach([](auto port) {
      port.template SetPullMode<mode, GetPinlistValueForPort(port, mask)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::SetSpeed(auto speed, typename PinList<_Pins...>::DataType mask)
  requires (PinList<_Pins...>::supports_speed) {
    static_assert(std::is_same_v<decltype(speed), typename PinList<_Pins...>::FirstPort::Speed>, "SetSpeed: argument type must match Port::Speed");
    _ports.foreach([mask, speed](auto port) {
      port.SetSpeed(speed, GetPinlistValueForPort(port, mask));
    });
  }

  template<typename... _Pins>
  template<auto speed, typename PinList<_Pins...>::DataType mask>
  void PinList<_Pins...>::SetSpeed()
  requires (PinList<_Pins...>::supports_speed) {
    static_assert(std::is_same_v<decltype(speed), typename PinList<_Pins...>::FirstPort::Speed>, "SetSpeed: template argument type must match Port::Speed");
    _ports.foreach([](auto port) {
      port.template SetSpeed<speed, GetPinlistValueForPort(port, mask)>();
    });
  }

  template<typename... _Pins>
  void PinList<_Pins...>::AltFuncNumber(uint8_t number, typename PinList<_Pins...>::DataType mask) {
    _ports.foreach([mask, number](auto port) {
      port.AltFuncNumber(number, GetPinlistValueForPort(port, mask));
    });
  }

  template<typename... _Pins>
  template<uint8_t number, typename PinList<_Pins...>::DataType mask>
  void PinList<_Pins...>::AltFuncNumber() {
    _ports.foreach([](auto port) {
      port.template AltFuncNumber<number, GetPinlistValueForPort(port, mask)>();
    });
  }

  template<typename... _Pins>
  constexpr auto PinList<_Pins...>::GetPinlistValueForPort(auto port, typename PinList<_Pins...>::DataType value) {
    auto result = typename template_utils::type_unbox<port>::DataType{};
    GetPinsForPort(port).foreach([value, &result](auto pin) {
      if (value & (1 << _pins.search(pin)))
        result |= (1 << pin.Number);
    });
    return result;
  }

  template<typename... _Pins>
  consteval auto PinList<_Pins...>::GetPinlistMaskForPort(auto port) {
    auto mask = typename template_utils::type_unbox<port>::DataType{};
    GetPinsForPort(port).foreach([&mask](auto pin) {
      mask |= (1 << pin.Number);
    });
    return mask;
  }

  template<typename... _Pins>
  template<typename Port>
  consteval auto PinList<_Pins...>::GetPinsForPort(template_utils::type_box<Port>) {
    return _pins.filter([](auto pin) {
      return std::is_same_v<typename template_utils::type_unbox<pin>::Port, Port>;
    });
  }

  template<typename... _Pins>
  typename PinList<_Pins...>::DataType PinList<_Pins...>::ExtractPinlistOutValueFromPort(auto port) {
    auto result = DataType{};
    auto portReadValue = template_utils::type_unbox<port>::Read();
    GetPinsForPort(port).foreach([&result, portReadValue](auto pin) {
      result |= ((portReadValue & (1 << pin.Number)) != 0)
        ? (1 << _pins.search(pin))
        : 0;
    });
    return result;
  }

  template<typename... _Pins>
  typename PinList<_Pins...>::DataType PinList<_Pins...>::ExtractPinlistValueFromPort(auto port) {
    auto result = DataType{};
    auto portReadValue = template_utils::type_unbox<port>::PinRead();
    GetPinsForPort(port).foreach([&result, portReadValue](auto pin) {
      result |= ((portReadValue & (1 << pin.Number)) != 0)
        ? (1 << _pins.search(pin))
        : 0;
    });
    return result;
  }

} // namespace Zhele::IO


