 /**
 * @file
 * Pinlist methods implementation
 * 
 * @author Aleksei Zhelonkin (based on Konstantin Chizhov)
 * @date 2019
 * @licence FreeBSD
 */
#ifndef ZHELE_PINLIST_IMPL_COMMON_H
#define ZHELE_PINLIST_IMPL_COMMON_H

using namespace Zhele::TemplateUtils;
namespace Zhele::IO
{
    template<typename... _Pins>
    void PinList<_Pins...>::Write(PinList<_Pins...>::DataType value)
    {
        _ports.foreach([value](auto port) {
            if constexpr (GetPinsForPort(port).size() == 16)
                port.Write(GetPinlistValueForPort(port, value));
            else 
                port.ClearAndSet(GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Write()
    {
        _ports.foreach([](auto port) {
            if constexpr (GetPinsForPort(port).size() == 16)
                port.template Write<GetPinlistValueForPort(port, value)>();
            else 
                port.template ClearAndSet<GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value)>();
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::Read()
    {
        auto result = DataType();

        _ports.foreach([&result](auto port){
            result |= ExtractPinlistOutValueFromPort(port);
        });

        return result;
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Clear(DataType value)
    {
        _ports.foreach([value](auto port){
            port.Clear(GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Clear()
    {
        _ports.foreach([](auto port){
            port.template Clear<GetPinlistValueForPort(port, value)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Set(PinList<_Pins...>::DataType value)
    {
        _ports.foreach([value](auto port){
            port.Set(GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Set()
    {
        _ports.foreach([](auto port){
            port.template Set<GetPinlistValueForPort(port, value)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::ClearAndSet(DataType clearMask, DataType setMask)
    {
        _ports.foreach([clearMask, setMask](auto port){
            port.ClearAndSet(GetPinlistValueForPort(port, clearMask), GetPinlistValueForPort(port, setMask));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType clearMask, typename PinList<_Pins...>::DataType setMask>
    void PinList<_Pins...>::ClearAndSet()
    {
        _ports.foreach([](auto port){
            port.template ClearAndSet<GetPinlistValueForPort(port, clearMask), GetPinlistValueForPort(port, setMask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Toggle(PinList<_Pins...>::DataType value)
    {
        _ports.foreach([value](auto port){
            port.Toggle(GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Toggle()
    {
        _ports.foreach([](auto port){
            port.template Toggle<GetPinlistValueForPort(port, value)>();
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::PinRead()
    {
        auto result = DataType();

        _ports.foreach([&result](auto port){
            result |= ExtractPinlistValueFromPort(port);
        });

        return result;
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetConfiguration(NativePortBase::Configuration config, PinList<_Pins...>::DataType mask)
    {
        _ports.foreach([mask, config](auto port){
            port.SetConfiguration(config, GetPinlistValueForPort(port, mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Configuration config, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetConfiguration()
    {
        _ports.foreach([](auto port){
            port.template SetConfiguration<config, GetPinlistValueForPort(port, mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetDriverType(NativePortBase::DriverType driverType, PinList<_Pins...>::DataType mask)
    {
        _ports.foreach([mask, driverType](auto port){
            port.SetDriverType(driverType, GetPinlistValueForPort(port, mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::DriverType driverType, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetDriverType()
    {
        _ports.foreach([](auto port){
            port.template SetDriverType<driverType, GetPinlistValueForPort(port, mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetPullMode(NativePortBase::PullMode pullMode, PinList<_Pins...>::DataType mask)
    {
        _ports.foreach([mask, pullMode](auto port){
            port.SetPullMode(pullMode, GetPinlistValueForPort(port, mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::PullMode pullMode, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetPullMode()
    {
        _ports.foreach([](auto port){
            port.template SetPullMode<pullMode, GetPinlistValueForPort(port, mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetSpeed(NativePortBase::Speed speed, PinList<_Pins...>::DataType mask)
    {
        _ports.foreach([mask, speed](auto port){
            port.SetSpeed(speed, GetPinlistValueForPort(port, mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Speed speed, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetSpeed()
    {
        _ports.foreach([](auto port){
            port.template SetSpeed<speed, GetPinlistValueForPort(port, mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::AltFuncNumber(uint8_t number, PinList<_Pins...>::DataType mask)
    {
        _ports.foreach([mask, number](auto port){
            port.AltFuncNumber(number, GetPinlistValueForPort(port, mask));
        });
    }

    template<typename... _Pins>
    template<uint8_t number, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::AltFuncNumber()
    {
        _ports.foreach([](auto port){
            port.template AltFuncNumber<number, GetPinlistValueForPort(port, mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Enable()
    {
        _ports.foreach([](auto port){ port.Enable(); });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Disable()
    {
        _ports.foreach([](auto port){ port.Disable(); });
    }

    template<typename... _Pins>
    constexpr auto PinList<_Pins...>::GetPinlistValueForPort(auto port, typename PinList<_Pins...>::DataType value)
    {
        auto result = typename TypeUnbox<port>::DataType();

        GetPinsForPort(port).foreach([value, &result](auto pin) {
            if (value & (1 << _pins.search(pin)))
                result |= (1 << pin.Number);
        });
        return result;
    }

    template<typename... _Pins>
    consteval auto PinList<_Pins...>::GetPinlistMaskForPort(auto port)
    {
        auto mask = typename TypeUnbox<port>::DataType();

        GetPinsForPort(port).foreach([&mask](auto pin) {
            mask |= (1 << pin.Number);
        });

        return mask;
    }

    template<typename... _Pins>
    template<typename Port>
    consteval auto PinList<_Pins...>::GetPinsForPort(TypeBox<Port> port)
    {
        return _pins.filter([](auto pin) {
            return std::is_same_v<typename TypeUnbox<pin>::Port, Port>;
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::ExtractPinlistOutValueFromPort(auto port)
    {
        auto result = DataType();
        auto portReadValue = TypeUnbox<port>::Read();
        
        GetPinsForPort(port).foreach([&result, portReadValue](auto pin){
            result |= ((portReadValue & (1 << pin.Number)) != 0)
                ? (1 << _pins.search(pin))
                : 0;
        });

        return result;
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::ExtractPinlistValueFromPort(auto port)
    {
        auto result = DataType();
        auto portReadValue = TypeUnbox<port>::PinRead();
        
        GetPinsForPort(port).foreach([&result, portReadValue](auto pin){
            result |= ((portReadValue & (1 << pin.Number)) != 0)
                ? (1 << _pins.search(pin))
                : 0;
        });

        return result;
    }
    
}
#endif //! ZHELE_PINLIST_IMPL_COMMON_H