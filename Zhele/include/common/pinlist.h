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

#include <ioports.h>

#include "template_utils/type_list.h"
#include "template_utils/data_type_selector.h"

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
        static constexpr auto _pins = TypeList<_Pins...>{};
        static constexpr auto _ports = TypeList<typename _Pins::Port ...>::remove_duplicates();

        // static checks
        static_assert(!_pins.is_empty(), "Pinlist cannot be empty");
        static_assert(_ports.is_unique(), "Fail to remove port duplicates!");

    public:
        /**
         * @brief Enables all used ports
         * 
         * @par Returns
         *	Nothing
            */
        static void Enable();

        /**
         * @brief Write value to pinlist
         * 
         * @param [in] value Value
         * 
         * @par Returns
         *	Nothing
            */
        static void Write(DataType value);

        /**
         * @brief Template version of Write method
         * 
         * @tparam value Value
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType value>
        static void Write();

        /**
         * @brief Read pinlist value
         * 
         * @todo Implement this method
         * 
         * @returns Pinlist input value
         */
        static DataType Read();

        /**
         * @brief Set pinlist with mask
         * 
         * @param [in] value Set mask
         * 
         * @par Returns
         * 	Nothing
         */
        static void Set(DataType value);

        /**
         * @brief Clear pinlist with mask
         * 
         * @param [in] value Clear mask
         * 
         * @par Returns
         * 	Nothing
         */
        static void Clear(DataType value);

        /**
         * @brief Set configuration with mask
         * 
         * @param [in] mask Mask
         * @param [in] config Configuration
         * 
         * @par Returns
         * 	Nothing
         */
        static void SetConfiguration(DataType mask, Configuration config);

        /**
         * @brief Set configuration with mask (template method)
         * 
         * @tparam [in] mask Mask
         * @tparam [in] config Configuration
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType mask, Configuration config>
        static void SetConfiguration();

        /**
         * @brief Set configuration
         * 
         * @tparam config Configuration
         * 
         * @par Returns
         * 	Nothing
         */
        template<Configuration config>
        static void SetConfiguration();

        /**
         * @brief Set speed with mask
         * 
         * @param [in] mask Mask
         * @param [in] speed Speed
         * 
         * @par Returns
         * 	Nothing
         */
        static void SetSpeed(DataType mask, Speed speed);

        /**
         * @brief Set speed with mask (template method)
         * 
         * @tparam mask Mask
         * @tparam speed Speed
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType mask, Speed speed>
        static void SetSpeed();

        /**
         * @brief Set speed
         * 
         * @tparam speed Speed
         * 
         * @par Returns
         * 	Nothing
         */
        template<Speed speed>
        static void SetSpeed();

        /**
         * @brief Set pull mode with mask
         * 
         * @param [in] mask Mask
         * @param [in] pull Pull mode
         * 
         * @par Returns
         * 	Nothing
         */
        static void SetPullMode(DataType mask, PullMode pullMode);

        /**
         * @brief Set pull mode with mask (template method)
         * 
         * @tparam mask Mask
         * @tparam pull Pull mode
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType mask, PullMode pullMode>
        static void SetPullMode();

        /**
         * @brief Set pull mode
         * 
         * @tparam pull Pull mode
         * 
         * @par Returns
         * 	Nothing
         */
        template<PullMode pullMode>
        static void SetPullMode();

        /**
         * @brief Set driver type with mask
         * 
         * @param [in] mask Mask
         * @param [in] driver Driver type
         * 
         * @par Returns
         * 	Nothing
         */
        static void SetDriverType(DataType mask, DriverType driverType);

        /**
         * @brief Set driver type with mask (template method)
         * 
         * @tparam mask Mask
         * @tparam driver Driver type
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType mask, DriverType driverType>
        static void SetDriverType();

        /**
         * @brief Set driver type
         * 
         * @tparam driver Driver type
         * 
         * @par Returns
         * 	Nothing
         */
        template<DriverType driverType>
        static void SetDriverType();

        /**
         * @brief Set alternate function number with mask
         * 
         * @param [in] mask Mask
         * @param [in] number Alternate function number
         * 
         * @par Returns
         * 	Nothing
         */
        static void AltFuncNumber(DataType mask, uint8_t number);

        /**
         * @brief Set alternate function number with mask (template method)
         * 
         * @tparam mask Mask
         * @tparam number Alternate function number
         * 
         * @par Returns
         * 	Nothing
         */
        template<DataType mask, unsigned number>
        static void AltFuncNumber();

        /**
         * @brief Set alternate function number
         * 
         * @tparam number Alternate function number
         * 
         * @par Returns
         * 	Nothing
         */
        template<unsigned number>
        static void AltFuncNumber();

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
        using Pin = TypeUnbox<_pins.template get<Index>()>;

    private:
        static constexpr auto GetPinlistValueForPort(auto port, DataType value);

        static consteval auto GetPinlistMaskForPort(auto port);

        template<typename Port>
        static consteval auto GetPinsForPort(TypeBox<Port> port);

        static DataType ExtractPinlistValueFromPort(auto port);
    };
} // namespace Zhele::IO

#include "impl/pinlist.h"

#endif //! ZHELE_PINLIST_COMMON_H