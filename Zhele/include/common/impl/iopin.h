/**
 * @file
 * IOPin methods implementation
 * 
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD 
 */
#ifndef ZHELE_IOPIN_IMPL_COMMON_H
#define ZHELE_IOPIN_IMPL_COMMON_H

namespace Zhele::IO
{
	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::Set()
	{
		Set(true);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::Set(bool state)
	{
		if (state)
			Port::template Set<1u << _Pin>();
		else
			Port::template Clear<1u << _Pin>();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetDir(bool isWrite)
	{
		isWrite
			? SetDirWrite()
			: SetDirRead();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::Clear()
	{
		Set(false);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::Toggle()
	{
		Port::Toggle(1u << _Pin);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetDirRead()
	{
		Port::template SetPinConfiguration<_Pin>(Port::In);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetDirWrite()
	{
		_ConfigPort:: template SetConfiguration<1u << _Pin, _Port::Configuration::Out>();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetConfiguration(Configuration configuration)
	{
		_ConfigPort::SetConfiguration(1u << _Pin, configuration);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	template<typename TPin<_Port, _Pin, _ConfigPort>::Configuration configuration>
	void TPin<_Port, _Pin, _ConfigPort>::SetConfiguration()
	{
		_ConfigPort::template SetConfiguration<1u << _Pin, configuration>();
	}
	
	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetDriverType(DriverType driverType)
	{
		_ConfigPort::SetDriverType(1u << _Pin, driverType);
	}
	
	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	template<typename TPin<_Port, _Pin, _ConfigPort>::DriverType driverType>
	void TPin<_Port, _Pin, _ConfigPort>::SetDriverType()
	{
		_ConfigPort::template SetDriverType<1u << _Pin, driverType>();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetPullMode(PullMode pullMode)
	{
		_ConfigPort::SetPullMode(1u << _Pin, pullMode);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	template<typename TPin<_Port, _Pin, _ConfigPort>::PullMode pullMode>
	void TPin<_Port, _Pin, _ConfigPort>::SetPullMode()
	{
		_ConfigPort::template SetPullMode<1u << _Pin, pullMode>();
	}
	
	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::SetSpeed(Speed speed)
	{
		_ConfigPort::SetSpeed(1u << _Pin, speed);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	template<typename TPin<_Port, _Pin, _ConfigPort>::Speed speed>
	void TPin<_Port, _Pin, _ConfigPort>::SetSpeed()
	{
		_ConfigPort::template SetSpeed<1u << _Pin, speed>();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::AltFuncNumber(uint8_t funcNumber)
	{
		_ConfigPort::AltFuncNumber(1u << _Pin, funcNumber);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	template<uint8_t funcNumber>
	void TPin<_Port, _Pin, _ConfigPort>::AltFuncNumber()
	{
		_ConfigPort::template AltFuncNumber<1u << _Pin, funcNumber>();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	bool TPin<_Port, _Pin, _ConfigPort>::IsSet()
	{
		return (Port::PinRead() & (typename Port::DataType)(1u << _Pin)) != 0;
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::WaitForSet()
	{
		while(IsSet()==0){}
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void TPin<_Port, _Pin, _ConfigPort>::WaitForClear()
	{
		while(IsSet()){}
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void InvertedPin<_Port, _Pin, _ConfigPort>::Set(bool val)
	{
		TPin<_Port, _Pin, _ConfigPort>::Set(!val);
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void InvertedPin<_Port, _Pin, _ConfigPort>::Set()
	{
		TPin<_Port, _Pin, _ConfigPort>::Clear();
	}

	template<typename _Port, uint8_t _Pin, typename _ConfigPort>
	void InvertedPin<_Port, _Pin, _ConfigPort>::Clear()
	{
		TPin<_Port, _Pin, _ConfigPort>::Set();
	}
}

#endif //! ZHELE_IOPIN_IMPL_COMMON_H