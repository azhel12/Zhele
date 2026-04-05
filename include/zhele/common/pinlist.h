 /**
 * @file
 * Contains class for pins list (~virtual port)
 * 
 * @author Aleksei Zhelonkin (based on Konstantin Chizhov)
 * @date 2019
 * @licence FreeBSD
 */
#ifndef ZHELE_PINLIST_COMMON_H
#define ZHELE_PINLIST_COMMON_H

#include "template_utils/data_type_selector.h"
#include "template_utils/mp_utils.h"

#include <zhele/ioports.h>

#include <algorithm>
#include <type_traits>

// TODO: Remove this later
using namespace Zhele::TemplateUtils;

namespace Zhele::IO
{
    /**
     * @brief Implements pin set and methods for manipulation with it.
     * 
     * @details
     * Pins from list are grouped by their port and group read/write operation is
     * performed on each port.
     */
    template<typename... _Pins>
    class PinList : public IO::NativePortBase
    {
        static constexpr auto _pins = std::array{mp::meta<_Pins>...};
        static constexpr auto _ports = mp::unique(std::array{mp::meta<typename _Pins::Port>...});

        // static checks
        static_assert(sizeof...(_Pins) > 0, "Pinlist cannot be empty");

    public:
        using DataType = mp::type_of<Zhele::TemplateUtils::template GetSuitableUnsignedType<_pins.size()>()>;
        /**
         * @brief Send value to port
         * 
         * @param [in] value Value to write
         * 
         * @par Returns
         *  Nothing
        */
        static void Write(DataType value);

        /**
         * @brief Template @ref Write method
         * 
         * @tparam value Value to write
         * 
         * @par Returns
         *  Nothing
        */
        template<DataType value>
        static void Write();

        /**
         * @brief Read output port value
         * @details
         * Method returns ODR register value
         * 
         * @return Port value
         */
        static DataType Read();

        /**
         * @brief Clear (reset) bits by mask
         * 
         * @param [in] value Clear mask
         * 
         * @par Returns
         *  Nothing
        */
        static void Clear(DataType value);

        /**
         * @brief Template @ref Clear method
         * 
         * @tparam value Clear mask
         * 
         * @par Returns
         *  Nothing
        */
        template<DataType value>
        static void Clear();

        /**
         * @brief Set bits by mask
         * 
         * @param [in] value Set mask
         * 
         * @par Returns
         *  Nothing
        */
        static void Set(DataType value);

        /**
         * @brief Template @ref Set method
        */
        template<DataType value>
        static void Set();

        /**
         * @brief Clear and set bits
         * 
         * @param [in] clearMask Clear mask
         * @param [in] setMask Set mask
         * 
         * @par Returns
         *  Nothing
        */
        static void ClearAndSet(DataType clearMask, DataType setMask);

        /**
         * @brief Template @ref ClearAndSet method
         * 
         * @tparam clearMask Clear mask
         * @tparam setMask Set mask
         * 
         * @par Returns
         *  Nothing
        */
        template<DataType clearMask, DataType setMask>
        static void ClearAndSet();

        /**
         * @brief Toggle output bits
         * 
         * @details Inverse output register
         * 
         * @param [in] value Toggle mask
         * 
         * @par Returns
         *  Nothing
        */
        static void Toggle(DataType value);

        /**
         * @brief Template @ref Toggle method
         * 
         * @tparam value Toggle mask
         * 
         * @par Returns
         *  Nothing
        */
        template<DataType value>
        static void Toggle();

        /**
         * @brief Returns input port value
         * 
         * @details
         * Read and return IDR register
         * 
         * @returns IDR register value
        */
        static DataType PinRead();

        /**
         * @brief Set port configuration
         * 
         * @param [in] mask Pin mask
         * @param [in] configuration Selected pins configuration
         * 
         * @par Returns
         *  Nothing
        */
        static void SetConfiguration(Configuration configuration, DataType mask = std::numeric_limits<DataType>::max());

        /**
         * @brief Template @ref SetConfiguration method
         * 
         * @tparam mask Pin mask
         * @tparam configuration Selected pins configuration
         * 
         * @par Returns
         *  Nothing
        */
        template<Configuration configuration, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetConfiguration();

        /**
         * @brief Set driver type
         * 
         * @param [in] mask Pin mask
         * @param [in] driver Driver type for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        static void SetDriverType(DriverType driver, DataType mask = std::numeric_limits<DataType>::max());

        /**
         * @brief Template @ref SetDriverType method
         * 
         * @tparam mask Pin mask
         * @tparam driver Driver type for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        template <DriverType driver, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetDriverType();

        /**
         * @brief Set pull smode
         * 
         * @param [in] mask Pin mask
         * @param [in] mode Pull mode for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        static void SetPullMode(PullMode mode, DataType mask = std::numeric_limits<DataType>::max());

        /**
         * @brief Template @ref SetPullMode method
         * 
         * @tparam mask Pin mask
         * @tparam mode Pull mode for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        template <PullMode mode, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetPullMode();

        /**
         * @brief Set port speed
         * 
         * @param [in] mask Pin mask
         * @param [in] speed Selected pins speed
         * 
         * @par Returns
         *  Nothing
        */
        static void SetSpeed(Speed speed, DataType mask = std::numeric_limits<DataType>::max());

        /**
         * @brief Template @ref SetSpeed method
         * 
         * @tparam mask Pin mask
         * @tparam speed Selected pins speed
         * 
         * @par Returns
         *  Nothing
        */
        template<Speed speed, DataType mask = std::numeric_limits<DataType>::max()>
        static void SetSpeed();

        /**
         * @brief Set alternate function number
         * 
         * @param [in] mask Pin mask
         * @param [in] number Alternate function number for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        static void AltFuncNumber(uint8_t number, DataType mask = std::numeric_limits<DataType>::max());

        /**
         * @brief Template @ref AltFuncNumber method
         * 
         * @tparam mask Pin mask
         * @tparam number Alternate function number for selected pins
         * 
         * @par Returns
         *  Nothing
        */
        template<uint8_t number, DataType mask = std::numeric_limits<DataType>::max()>
        static void AltFuncNumber();

        /**
         * @brief Enable clock for port
         * 
         * @par Returns
         *  Nothing
        */
        static void Enable();

        /**
         * @brief Disable clock for port
         * 
         * @par Returns
         *  Nothing
        */
        static void Disable();

        /**
         * @brief Detect index of pin in pinlist
         * 
         * @tparam Pin Pin
         */
        template<typename Pin>
        static constexpr int IndexOf = std::ranges::find(_pins, mp::meta<Pin>) == std::end(_pins)
            ? -1
            : std::ranges::distance(std::begin(_pins), std::ranges::find(_pins, mp::meta<Pin>));

        /**
         * @brief Returns pin by index
         * 
         * @tparam Index Index
         */
        template<int Index>
        using Pin = mp::type_of<Index >= 0 ? _pins[Index] : mp::meta<void>>;

    private:
        template<auto port>
        static constexpr auto GetPinlistValueForPort(typename PinList<_Pins...>::DataType value);

        template<auto port>
        static consteval auto GetPinlistMaskForPort();
        template<auto port>
        static consteval auto GetPinsForPort();

        template<auto port>
        static auto ExtractPinlistOutValueFromPort();
        template<auto port>
        static auto ExtractPinlistValueFromPort();
    };
} // namespace Zhele::IO

#include "impl/pinlist.h"

#endif //! ZHELE_PINLIST_COMMON_H