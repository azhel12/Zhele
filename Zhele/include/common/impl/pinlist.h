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
    namespace Private
    {
        template<typename... _PortPins>
        template<typename _PinList, typename _DataType>
        NativePortBase::DataType PinListExpander<TypeList<_PortPins...>>::ExpandPinlistValue(_DataType value)
        {
            return (((value & (1 << TypeIndex<_PortPins, _PinList>::value)) > 0 ? 1 << _PortPins::Number : 0) | ...);
        }

        template<typename... _PortPins>
        template<typename _PinList, typename _DataType>
        _DataType PinListExpander<TypeList<_PortPins...>>::ExtractPinlistValueFromPort(NativePortBase::DataType value)
        {
            return (((value & (1 << _PortPins::Number)) > 0 ? (1 << TypeIndex<_PortPins, _PinList>::value) : 0) | ...);
        }

        template<typename _Port, typename _PinList, typename _DataType>
        typename _Port::DataType GetPinlistValueForPort(_DataType value)
        {
            return PinListExpander<PinsForPort<_Port, _PinList>>::template ExpandPinlistValue<_PinList>(value);
        }

        template<typename _Port, typename _PinList, typename _DataType>
        _DataType GetPinlistValuePartFromPort()
        {
            return PinListExpander<PinsForPort<_Port, _PinList>>::template ExtractPinlistValueFromPort<_PinList, _DataType>(_Port::PinRead());
        }

        template<typename _PinList, typename... _Ports>
        template<typename _Port, typename _DataType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::WriteToOnePort(_DataType value)
        {
            if constexpr (Length<PinsForPort<_Port, _PinList>>::value == 16)
            {
                _Port::Send(Private::GetPinlistValueForPort<_Port, _PinList>(value));
            }
            else
            {
                _Port::ClearAndSet(PortMask<PinsForPort<_Port, _PinList>>::value, Private::GetPinlistValueForPort<_Port, _PinList>(value));
            }
        }

        template<typename _PinList, typename... _Ports>
        void PortsWriter<_PinList, TypeList<_Ports...>>::Enable()
        {
            (_Ports::Enable(), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::Write(_DataType value)
        {
            (WriteToOnePort<_Ports>(value), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType>
        _DataType PortsWriter<_PinList, TypeList<_Ports...>>::Read()
        {
            return (Private::GetPinlistValuePartFromPort<_Ports, _PinList, _DataType>() | ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::Set(_DataType value)
        {
            (_Ports::Set(Private::GetPinlistValueForPort<_Ports, _PinList>(value)), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::Clear(_DataType value)
        {
            (_Ports::Clear(Private::GetPinlistValueForPort<_Ports, _PinList>(value)), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType, typename Configuration>
        void PortsWriter<_PinList, TypeList<_Ports...>>::SetConfiguration(_DataType mask, Configuration config)
        {
            (_Ports::SetConfiguration(Private::GetPinlistValueForPort<_Ports, _PinList>(mask), config), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType, typename Speed>
        void PortsWriter<_PinList, TypeList<_Ports...>>::SetSpeed(_DataType mask, Speed speed)
        {
            (_Ports::SetSpeed(Private::GetPinlistValueForPort<_Ports, _PinList>(mask), speed), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType, typename PullMode>
        void PortsWriter<_PinList, TypeList<_Ports...>>::SetPullMode(_DataType mask, PullMode mode)
        {
            (_Ports::SetPullMode(Private::GetPinlistValueForPort<_Ports, _PinList>(mask), mode), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType, typename DriverType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::SetDriverType(_DataType mask, DriverType driver)
        {
            (_Ports::SetDriverType(Private::GetPinlistValueForPort<_Ports, _PinList>(mask), driver), ...);
        }

        template<typename _PinList, typename... _Ports>
        template<typename _DataType>
        void PortsWriter<_PinList, TypeList<_Ports...>>::AltFuncNumber(_DataType mask, uint8_t number)
        {
            (_Ports::AltFuncNumber(Private::GetPinlistValueForPort<_Ports, _PinList>(mask), number), ...);
        }
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Enable()
    {
        PortWriter::Enable();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Write(PinList<_Pins...>::DataType value)
    {
        PortWriter::Write(value);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType value>
    void PinList<_Pins...>::Write()
    {
        PortWriter::Write(value);
    }

    template<typename... _Pins>
    typename PinList<_Pins...>::DataType PinList<_Pins...>::Read()
    {
        return PortWriter::template Read<DataType>();
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Set(PinList<_Pins...>::DataType value)
    {
        PortWriter::Set(value);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::Clear(DataType value)
    {
        PortWriter::Clear(value);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetConfiguration(PinList<_Pins...>::DataType mask, NativePortBase::Configuration config)
    {
        PortWriter::SetConfiguration(mask, config);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::Configuration config>
    void PinList<_Pins...>::SetConfiguration()
    {
        PortWriter::SetConfiguration(mask, config);
    }

    template<typename... _Pins>
    template<NativePortBase::Configuration config>
    void PinList<_Pins...>::SetConfiguration()
    {
        PortWriter::SetConfiguration(-1, config);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetSpeed(PinList<_Pins...>::DataType mask, NativePortBase::Speed speed)
    {
        PortWriter::SetSpeed(mask, speed);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::Speed speed>
    void PinList<_Pins...>::SetSpeed()
    {
        PortWriter::SetSpeed(mask, speed);
    }

    template<typename... _Pins>
    template<NativePortBase::Speed speed>
    void PinList<_Pins...>::SetSpeed()
    {
        PortWriter::SetSpeed(-1, speed);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetPullMode(PinList<_Pins...>::DataType mask, NativePortBase::PullMode pull)
    {
        PortWriter::SetPullMode(mask, pull);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::PullMode pull>
    void PinList<_Pins...>::SetPullMode()
    {
        PortWriter::SetPullMode(mask, pull);
    }

    template<typename... _Pins>
    template<NativePortBase::PullMode pull>
    void PinList<_Pins...>::SetPullMode()
    {
        PortWriter::SetPullMode(-1, pull);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::SetDriverType(PinList<_Pins...>::DataType mask, NativePortBase::DriverType driver)
    {
        PortWriter::SetDriverType(mask, driver);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, NativePortBase::DriverType driver>
    void PinList<_Pins...>::SetDriverType()
    {
        PortWriter::SetDriverType(mask, driver);
    }

    template<typename... _Pins>
    template<NativePortBase::DriverType driver>
    void PinList<_Pins...>::SetDriverType()
    {
        PortWriter::SetDriverType(-1, driver);
    }

    template<typename... _Pins>
    void PinList<_Pins...>::AltFuncNumber(PinList<_Pins...>::DataType mask, uint8_t number)
    {
        PortWriter::AltFuncNumber(mask, number);
    }

    template<typename... _Pins>
    template<typename PinList<_Pins...>::DataType mask, unsigned number>
    void PinList<_Pins...>::AltFuncNumber()
    {
        PortWriter::AltFuncNumber(mask, number);
    }

    template<typename... _Pins>
    template<unsigned number>
    void PinList<_Pins...>::AltFuncNumber()
    {
        PortWriter::AltFuncNumber(-1, number);
    }
}
#endif //! ZHELE_PINLIST_IMPL_COMMON_H
