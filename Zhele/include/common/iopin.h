/**
 * @file
 * Implement GPIO pin class
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_IOPIN_COMMON_H
#define ZHELE_IOPIN_COMMON_H

#include <stdint.h>

namespace Zhele
{
	namespace IO
	{
		/**
		 * @brief Represents one pin in a IO port.
		 * 
		 * @details
		 * This fully-static class represent one GPIO pin
		 * 
		 * @tparam _Port Parent port
		 * @tparam _Pin Pin number (starts from zero)
		 * @tparam _ConfigPort Configuration port (default is equal to _Port)
		 * @CONFIG_PORT
		 */
		template<typename _Port, uint8_t _Pin, typename _ConfigPort = _Port>
		class TPin
		{
			static_assert(_Pin < 16);
		public:
			using Port = _Port;
			using Speed = typename _ConfigPort::Speed;
			using PullMode = typename _ConfigPort::PullMode;
			using DriverType = typename _ConfigPort::DriverType;
			using Configuration = typename _ConfigPort::Configuration;

			static const unsigned Number = _Pin;
			static const bool Inverted = false;

			/**
			 * @brief Set port (in active state)
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void Set();

			/**
			 * @brief Set port in given state
			 * 
			 * @param [in] state Port state. True - set, false - reset.
			 * 
			 * @per Returns
			 *	Nothing
			 */
			static void Set(bool state);
			/**
			 * @brief Set pin direction (read or write)
			 * 
			 * @param isWrite Direction. true - write, false - read
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetDir(bool isWrite);

			/**
			 * @brief Clear (reset) pin
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void Clear();

			/**
			 * @brief Toggle (invert) pin.
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void Toggle();

			/**
			 * @brief Set pin direction to read
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetDirRead();

			/**
			 * @brief Set pin direction to write
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetDirWrite();

			/**
			 * @brief Set pin configuration (In/Out/Analog/Alternate)
			 * 
			 * @param [in] configuration Port config (In or Out)
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetConfiguration(Configuration configuration);

			/**
			 * @brief Template variant of @ref SetConfiguration method
			 * 
			 * @tparam configuration Target configuration
			 */
			template<Configuration configuration>
			static void SetConfiguration();
			/**
			 * @brief Set pin driver type (push-pull or open-drain)
			 * 
			 * @param [in] driverType Driver Type
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetDriverType(DriverType driverType);

			/**
			 * @brief Template variant of @ref SetDriverType method
			 * 
			 * @tparam driverType Driver Type
			 * 
			 * @par Returns
			 *	Nothing
			 */
			template<DriverType driverType>
			static void SetDriverType();

			/**
			 * @brief Set pin pull type (Up/Down/Nopull)
			 * 
			 * @param [in] pullMode Pull type
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetPullMode(PullMode pullMode);

			/**
			 * @brief Template variant of @ref SetPullMode method
			 * 
			 * @tparam pullMode Pull type
			 * 
			 * @par Returns
			 *	Nothing
			 */
			template<PullMode pullMode>
			static void SetPullMode();

			/**
			 * @brief Set pin speed (Low/Medium/High)
			 * 
			 * @param [in] speed Pin Speed
			 */
			static void SetSpeed(Speed speed);

			/**
			 * @brief Template variant of @ref SetSpeed method
			 * 
			 * @tparam speed Pin Speed
			 */
			template <Speed speed> 
			static void SetSpeed();

			/**
			 * @brief Set pin alternate function
			 * 
			 * @param [in] funcNumber Alternative pin function number (see table in DataSheet)
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void AltFuncNumber(uint8_t funcNumber);

			/**
			 * @brief Template variant of @ref AltFuncNumber method
			 * 
			 * @param [in] funcNumber Alternative pin function number (see table in DataSheet)
			 * 
			 * @par Returns
			 *	Nothing
			 */
			template<uint8_t funcNumber>
			static void AltFuncNumber();

			/**
			 * @brief Check that pin is set now
			 * 
			 * @retval true PIN is in set state
			 * @retval false PIN is in reset state
			 */
			static bool IsSet();

			/**
			 * @brief Synchronous wait for pin set
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void WaitForSet();

			/**
			 * @brief Synchronous wait for pin reset
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void WaitForClear();
		};

		/**
		 * @brief Class represent inverted pin
		 * 
		 * @details
		 * Class is similar as class PIN, except for
		 * set state mean low level and vice versa
		 */
		template<typename _Port, uint8_t _Pin, typename _ConfigPort = _Port>
		class InvertedPin :public TPin<_Port, _Pin, _ConfigPort>
		{
		public:
			static const bool Inverted = true;

			static void Set(bool val);
			static void Set();
			static void Clear();
		};
	}
}
#include "impl/iopin.h"

#endif // !ZHELE_IOPIN_COMMON_H