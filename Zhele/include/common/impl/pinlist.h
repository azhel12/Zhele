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
    void PinList<_Pins...>::Enable()
    {
        _ports.foreach([](auto port){ TypeUnbox<port>::Enable(); });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Write(PinList<_Pins...>::DataType value)
    {
        _ports.foreach([value](auto port) {
            if constexpr (GetPinsForPort(port).size() == 16)
                TypeUnbox<port>::Write(GetPinlistValueForPort(port, value));
            else 
                TypeUnbox<port>::ClearAndSet(GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Write()
    {
        _ports.foreach([](auto port) {
            if constexpr (GetPinsForPort(port).size() == 16)
                TypeUnbox<port>::Write(GetPinlistValueForPort(port, value));
            else 
                TypeUnbox<port>::ClearAndSet(GetPinlistMaskForPort(port), GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::Read()
    {
        auto result = DataType();

        _ports.foreach([&result](auto port){
            result |= ExtractPinlistValueFromPort(port);
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Set(PinList<_Pins...>::DataType value)
    {
        _ports.foreach([value](auto port){
            TypeUnbox<port>::Set(GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Clear(DataType value)
    {
        _ports.foreach([value](auto port){
            TypeUnbox<port>::Clear(GetPinlistValueForPort(port, value));
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetConfiguration(PinList<_Pins...>::DataType mask, NativePortBase::Configuration config)
    {
        _ports.foreach([mask, config](auto port){
            TypeUnbox<port>::SetConfiguration(GetPinlistValueForPort(port, mask), config);
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::Configuration config>
    void PinList<_Pins...>::SetConfiguration()
    {
        _ports.foreach([](auto port){
            TypeUnbox<port>::template SetConfiguration<GetPinlistValueForPort(port, mask), config>();
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Configuration config>
    void PinList<_Pins...>::SetConfiguration()
    {
        SetConfiguration<std::numeric_limits<DataType>::max(), config>();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetSpeed(PinList<_Pins...>::DataType mask, NativePortBase::Speed speed)
    {
        _ports.foreach([mask, speed](auto port){
            TypeUnbox<port>::SetSpeed(GetPinlistValueForPort(port, mask), speed);
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::Speed speed>
    void PinList<_Pins...>::SetSpeed()
    {
        _ports.foreach([](auto port){
            TypeUnbox<port>::template SetSpeed<GetPinlistValueForPort(port, mask), speed>();
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Speed speed>
    void PinList<_Pins...>::SetSpeed()
    {
        SetSpeed<std::numeric_limits<DataType>::max(), speed>();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetPullMode(PinList<_Pins...>::DataType mask, NativePortBase::PullMode pullMode)
    {
        _ports.foreach([mask, pullMode](auto port){
            TypeUnbox<port>::SetPullMode(GetPinlistValueForPort(port, mask), pullMode);
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::PullMode pullMode>
    void PinList<_Pins...>::SetPullMode()
    {
        _ports.foreach([](auto port){
            TypeUnbox<port>::template SetPullMode<GetPinlistValueForPort(port, mask), pullMode>();
        });
    }

    template<typename... _Pins>
    template<NativePortBase::PullMode pullMode>
    void PinList<_Pins...>::SetPullMode()
    {
        SetPullMode<std::numeric_limits<DataType>::max(), pullMode>();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetDriverType(PinList<_Pins...>::DataType mask, NativePortBase::DriverType driverType)
    {
        _ports.foreach([mask, driverType](auto port){
            TypeUnbox<port>::SetDriverType(GetPinlistValueForPort(port, mask), driverType);
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::DriverType driverType>
    void PinList<_Pins...>::SetDriverType()
    {
        _ports.foreach([](auto port){
            TypeUnbox<port>::template SetDriverType<GetPinlistValueForPort(port, mask), driverType>();
        });
    }

    template<typename... _Pins>
    template<NativePortBase::DriverType driverType>
    void PinList<_Pins...>::SetDriverType()
    {
        SetDriverType<std::numeric_limits<DataType>::max(), driverType>();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::AltFuncNumber(PinList<_Pins...>::DataType mask, uint8_t number)
    {
        _ports.foreach([mask, number](auto port){
            TypeUnbox<port>::AltFuncNumber(GetPinlistValueForPort(port, mask), number);
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, unsigned number>
    void PinList<_Pins...>::AltFuncNumber()
    {
        _ports.foreach([](auto port){
            TypeUnbox<port>::template AltFuncNumber<GetPinlistValueForPort(port, mask), number>();
        });
    }

    template<typename... _Pins>
    template<unsigned number>
    void PinList<_Pins...>::AltFuncNumber()
    {
        AltFuncNumber<std::numeric_limits<DataType>::max(), number>();
    }

    template<typename... _Pins>
    constexpr auto PinList<_Pins...>::GetPinlistValueForPort(auto port, typename PinList<_Pins...>::DataType value)
    {
        auto result = typename TypeUnbox<port>::DataType();

        GetPinsForPort(port).foreach([value, &result](auto pin) {
            if (value & (1 << TypeUnbox<pin>::Number))
                result |= (1 << _pins.search(pin));
        });
        return result;
    }

    template<typename... _Pins>
    consteval auto PinList<_Pins...>::GetPinlistMaskForPort(auto port)
    {
        auto mask = typename TypeUnbox<port>::DataType();

        GetPinsForPort(port).foreach([&mask](auto pin) {
            mask |= (1 << TypeUnbox<pin>::Number);
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
    typename PinList<_Pins...>::DataType PinList<_Pins...>::ExtractPinlistValueFromPort(auto port)
    {
        auto result = DataType();
        auto portReadValue = TypeUnbox<port>::PinRead();
        
        GetPinsForPort(port).foreach([&result, portReadValue](auto pin){
            result |= ((portReadValue & (1 << TypeUnbox<pin>::Number)) != 0)
                ? (1 << _pins.search(pin))
                : 0;
        });

        return result;
    }
    
}
#endif //! ZHELE_PINLIST_IMPL_COMMON_H
