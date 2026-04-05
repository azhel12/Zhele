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

#include <ranges>

// TODO: Remove this
using namespace Zhele::TemplateUtils;

namespace Zhele::IO
{
    template<typename... _Pins>
    void PinList<_Pins...>::Write(PinList<_Pins...>::DataType value)
    {
        mp::for_each<_ports>([value]<mp::info port> {
            if constexpr (GetPinsForPort<port>().size() == std::numeric_limits<typename mp::type_of<port>::DataType>::digits) {
                mp::type_of<port>::Write(GetPinlistValueForPort<port>(value));
            } else  {
                mp::type_of<port>::ClearAndSet(GetPinlistMaskForPort<port>(), GetPinlistValueForPort<port>(value));
            }
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Write()
    {
        mp::for_each<_ports>([]<mp::info port> {
            if constexpr (GetPinsForPort<port>().size() == std::numeric_limits<typename mp::type_of<port>::DataType>::digits) {
                mp::type_of<port>::template Write<GetPinlistValueForPort<port>(value)>();
            }
            else {
                mp::type_of<port>::template ClearAndSet<GetPinlistMaskForPort<port>(), GetPinlistValueForPort<port>(value)>();
            }
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::Read()
    {
        DataType result{};

        mp::for_each<_ports>([&result]<mp::info port> {
            result |= ExtractPinlistOutValueFromPort<port>();
        });

        return result;
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Clear(DataType value)
    {
        mp::for_each<_ports>([value]<mp::info port> {
            mp::type_of<port>::Clear(GetPinlistValueForPort<port>(value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Clear()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template Clear<GetPinlistValueForPort<port>(value)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Set(PinList<_Pins...>::DataType value)
    {
        mp::for_each<_ports>([value]<mp::info port> {
            mp::type_of<port>::Set(GetPinlistValueForPort<port>(value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Set()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template Set<GetPinlistValueForPort<port>(value)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::ClearAndSet(DataType clearMask, DataType setMask)
    {
        mp::for_each<_ports>([clearMask, setMask]<mp::info port> {
            mp::type_of<port>::ClearAndSet(GetPinlistValueForPort<port>(clearMask), GetPinlistValueForPort<port>(setMask));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType clearMask, typename PinList<_Pins...>::DataType setMask>
    void PinList<_Pins...>::ClearAndSet()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template ClearAndSet<GetPinlistValueForPort<port>(clearMask), GetPinlistValueForPort<port>(setMask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Toggle(PinList<_Pins...>::DataType value)
    {
        mp::for_each<_ports>([value]<mp::info port> {
            mp::type_of<port>::Toggle(GetPinlistValueForPort<port>(value));
        });
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Toggle()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template Toggle<GetPinlistValueForPort<port>(value)>();
        });
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::PinRead()
    {
        DataType result{};

        mp::for_each<_ports>([&result]<mp::info port> {
            result |= ExtractPinlistValueFromPort<port>();
        });

        return result;
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetConfiguration(NativePortBase::Configuration config, PinList<_Pins...>::DataType mask)
    {
        mp::for_each<_ports>([mask, config]<mp::info port> {
            mp::type_of<port>::SetConfiguration(config, GetPinlistValueForPort<port>(mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Configuration config, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetConfiguration()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template SetConfiguration<config, GetPinlistValueForPort<port>(mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetDriverType(NativePortBase::DriverType driverType, PinList<_Pins...>::DataType mask)
    {
        mp::for_each<_ports>([mask, driverType]<mp::info port> {
            mp::type_of<port>::SetDriverType(driverType, GetPinlistValueForPort<port>(mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::DriverType driverType, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetDriverType()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template SetDriverType<driverType, GetPinlistValueForPort<port>(mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetPullMode(NativePortBase::PullMode pullMode, PinList<_Pins...>::DataType mask)
    {
        mp::for_each<_ports>([mask, pullMode]<mp::info port> {
            mp::type_of<port>::SetPullMode(pullMode, GetPinlistValueForPort<port>(mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::PullMode pullMode, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetPullMode()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template SetPullMode<pullMode, GetPinlistValueForPort<port>(mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetSpeed(NativePortBase::Speed speed, PinList<_Pins...>::DataType mask)
    {
        mp::for_each<_ports>([mask, speed]<mp::info port> {
            mp::type_of<port>::SetSpeed(speed, GetPinlistValueForPort<port>(mask));
        });
    }

    template<typename... _Pins>
    template<NativePortBase::Speed speed, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::SetSpeed()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template SetSpeed<speed, GetPinlistValueForPort<port>(mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::AltFuncNumber(uint8_t number, PinList<_Pins...>::DataType mask)
    {
        mp::for_each<_ports>([mask, number]<mp::info port> {
            mp::type_of<port>::AltFuncNumber(number, GetPinlistValueForPort<port>(mask));
        });
    }

    template<typename... _Pins>
    template<uint8_t number, typename PinList<_Pins...>::DataType mask>
    void PinList<_Pins...>::AltFuncNumber()
    {
        mp::for_each<_ports>([]<mp::info port> {
            mp::type_of<port>::template AltFuncNumber<number, GetPinlistValueForPort<port>(mask)>();
        });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Enable()
    {
        mp::for_each<_ports>([]<mp::info port> { mp::type_of<port>::Enable(); });
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Disable()
    {
        mp::for_each<_ports>([]<mp::info port> { mp::type_of<port>::Disable(); });
    }
    
    template<typename... _Pins>
    template<auto port>
    constexpr auto PinList<_Pins...>::GetPinlistValueForPort(typename PinList<_Pins...>::DataType value)
    {
        DataType result{};

        mp::for_each<GetPinsForPort<port>()>([value, &result]<mp::info pin> {
            constexpr auto pin_in_list = std::ranges::find(_pins, pin);
            if (value & (1 << std::ranges::distance(_pins.begin(), pin_in_list)))
                result |= (1 << mp::type_of<pin>::Number);
        });
        return result;
    }

    template<typename... _Pins>
    template<auto port>
    consteval auto PinList<_Pins...>::GetPinlistMaskForPort()
    {
        DataType mask{};

        mp::for_each<GetPinsForPort<port>()>([&mask]<mp::info pin> {
            mask |= (1 << mp::type_of<pin>::Number);
        });

        return mask;
    }

    template<typename... _Pins>
    template<auto port>
    consteval auto PinList<_Pins...>::GetPinsForPort()
    {
        mp::vector<mp::info> out;
        mp::unroll<_pins.size()>([&out]<std::size_t I> {
            if constexpr (std::is_same_v<typename mp::type_of<_pins[I]>::Port, typename mp::type_of<port>>)
                out.push_back(_pins[I]);
        });
        return out;
    }

    template<typename... _Pins>
    template<auto port>
    auto PinList<_Pins...>::ExtractPinlistOutValueFromPort()
    {
        auto result = DataType();
        auto portReadValue = mp::type_of<port>::Read();

        mp::for_each<GetPinsForPort<port>()>([&result, portReadValue]<mp::info pin> {
            auto pin_in_list = std::ranges::find(_pins, pin);
            result |= ((portReadValue & (1 << mp::type_of<pin>::Number)) != 0)
                ? (1 << std::ranges::distance(_pins.begin(), pin_in_list))
                : 0;
        });

        return result;
    }

    template<typename... _Pins>
    template<auto port>
    auto PinList<_Pins...>::ExtractPinlistValueFromPort()
    {
        auto result = DataType();
        auto portReadValue = mp::type_of<port>::PinRead();

        mp::for_each<GetPinsForPort<port>()>([&result, portReadValue]<mp::info pin> {
            auto pin_in_list = std::ranges::find(_pins, pin);
            result |= ((portReadValue & (1 << mp::type_of<pin>::Number)) != 0)
                ? (1 << std::ranges::distance(_pins.begin(), pin_in_list))
                : 0;
        });

        return result;
    }
    
}
#endif //! ZHELE_PINLIST_IMPL_COMMON_H