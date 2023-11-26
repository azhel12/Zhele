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

#include "template_utils/type_list.h"
#include "template_utils/data_type_selector.h"

#include <zhele/ioports.h>

#include <type_traits>
#include <numeric>
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
        static constexpr auto _pins = Zhele::TemplateUtils::TypeList<_Pins...>{};
        static constexpr auto _ports = Zhele::TemplateUtils::TypeList<typename _Pins::Port ...>::remove_duplicates();

        // static checks
        static_assert(!_pins.is_empty(), "Pinlist cannot be empty");
        static_assert(_ports.is_unique(), "Fail to remove port duplicates!");

    public:
        using DataType = Zhele::TemplateUtils::TypeUnbox<GetSuitableUnsignedType<_pins.size()>()>;
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
        const static int IndexOf = _pins.template search<Pin>();

        /**
         * @brief Returns pin by index
         * 
         * @tparam Index Index
         */
        template<int Index>
        using Pin = Zhele::TemplateUtils::TypeUnbox<_pins.template get<Index>()>;

    private:
        static constexpr auto GetPinlistValueForPort(auto port, DataType value);

        static consteval auto GetPinlistMaskForPort(auto port);

        template<typename Port>
        static consteval auto GetPinsForPort(TypeBox<Port> port);

        static DataType ExtractPinlistOutValueFromPort(auto port);
        static DataType ExtractPinlistValueFromPort(auto port);
    };
} // namespace Zhele::IO

#include "impl/pinlist.h"

#endif //! ZHELE_PINLIST_COMMON_H