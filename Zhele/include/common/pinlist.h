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

using namespace Zhele::TemplateUtils;

namespace Zhele
{
    namespace IO
    {
        namespace Private
        {
            /**
             * @brief Predicate for get pins of given port
             * @details
             * Template utils allows to make type list from other
             * by some predicate. This predicate is used for get all pins from
             * pin list with one port.
             */
            template<typename _ExpectedPort>
            class IsSamePort
            {
            public:
                template<typename _Pin>
                class type : public std::is_same<_ExpectedPort, typename _Pin::Port>
                {
                };
            };

            /**
             * @brief Pins from pin list for given port
             */
            template<typename _Port, typename _PinList>
            using PinsForPort = typename Sample<IsSamePort<_Port>::template type, _PinList>::type;

            /**
             * @brief Mask for port pins
             */
            template<typename...>
            class PortMask {};

            template<typename... _Pins>
            class PortMask<TypeList<_Pins...>>
            {
            public:
                static const unsigned int value = ((1 << _Pins::Number) | ...);
            };

            /**
             * @brief Class for convert values from value for pinlist to values for each port
             */
            template<typename...>
            class PinListExpander {};
            template<typename... _PortPins>
            class PinListExpander<TypeList<_PortPins...>>
            {
            public:
                /**
                 * @brief Expand pinlist value to one port's value (ODR)
                 * 
                 * @tparam _PinList Pinlist
                 * @tparam _DataType Data type
                 * 
                 * @param value Pinlist value
                 * 
                 * @return NativePortBase::DataType value for specific port
                 */
                template<typename _PinList, typename _DataType>
                static NativePortBase::DataType ExpandPinlistValue(_DataType value);

                /**
                 * @brief Extact pinlist value part from one port's value (IDR)
                 * 
                 * @tparam _PinList Pinlist
                 * @tparam _DataType Data type
                 * 
                 * @param value Port value
                 * 
                 * @return _DataType Pinlist part value (for next OR operation)
                 */
                template<typename _PinList, typename _DataType>
                static _DataType ExtractPinlistValueFromPort(NativePortBase::DataType value);
            };

            /**
             * @brief Get target port`s part of pinlist value
             */
            template<typename _Port, typename _PinList, typename _DataType>
            typename _Port::DataType GetPinlistValueForPort(_DataType value);

            /**
             * @brief Get pinlist's part from port value
             */
            template<typename _Port, typename _PinList, typename _DataType>
            _DataType GetPinlistValuePartFromPort();

            /**
             * @brief Writer to ports
             * @details
             * In pinlist class fold Pins::Port gives ports with duplicates, but
             * it is needlessly to make actions with one port more than one times;
             * So, this class has a partial specialization for TypeList<UsedPorts...>
             * It is allows to makes actions with Unique<Pins::UsedPorts...>
             * 
             */
            template<typename...>
            class PortsWriter {};

            template<typename _PinList, typename... _Ports>
            class PortsWriter<_PinList, TypeList<_Ports...>>
            {

                /**
                 * @brief Writes value to port
                 * 
                 * @tparam [in] _Port Port
                 * @tparam [in] _DataType Pinlist data type
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _Port, typename _DataType>
                static void WriteToOnePort(_DataType value);
                
            public:
                /**
                 * @brief Enable all ports
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void Enable();
                

                /**
                 * @brief Write value to pinlist
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType>
                static void Write(_DataType value);
                

                /**
                 * @brief Read value from pinlist
                 * 
                 * @todo Implement this method!
                 * 
                 * @returns Pinlist input value
                 */
                template<typename _DataType>
                static _DataType Read();
                

                /**
                 * @brief Set value with mask
                 * 
                 * @param [in] value Set mask
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType>
                static void Set(_DataType value);
                

                /**
                 * @brief Clear pinlist with mask
                 * 
                 * @param [in] value Clear mask
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType>
                static void Clear(_DataType value);
                

                /**
                 * @brief Set configuration with mask
                 * 
                 * @param [in] mask Mask
                 * @param [in] config Configuration
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType, typename Configuration>
                static void SetConfiguration(_DataType mask, Configuration config);
                

                /**
                 * @brief Set speed with mask
                 * 
                 * @param [in] mask Mask
                 * @param [in] speed Speed
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType, typename Speed>
                static void SetSpeed(_DataType mask, Speed speed);
                

                /**
                 * @brief Set pull mode with mask
                 * 
                 * @param [in] mask Mask
                 * @param [in] speed Speed
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType, typename PullMode>
                static void SetPullMode(_DataType mask, PullMode mode);
                

                /**
                 * @brief Set driver type with mask
                 * 
                 * @param [in] mask Mask
                 * @param [in] speed Speed
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType, typename DriverType>
                static void SetDriverType(_DataType mask, DriverType driver);
            

                /**
                 * @brief Set alternative function number with mask
                 * 
                 * @param [in] mask Mask
                 * @param [in] speed Speed
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename _DataType>
                static void AltFuncNumber(_DataType mask, uint8_t number);
                
            };
        }
        /**
         * @brief Class represent properties for pin list
         */
        template<typename... _Pins>
        class PinListProperties
        {	
            static const unsigned Length = sizeof...(_Pins);
        };

        /**
         * @brief Implements pin set and methods for manipulation with it.
         * 
         * @details
         * Pins from list are grouped by their port and group read/write operation is
         * performed on each port.
         */
        template<typename... _Pins>
        class PinList : public PinListProperties<_Pins...>, public IO::NativePortBase
        {
            using Config = PinListProperties<_Pins...>;
            // Used ports (unique list)
            using UsedPorts = typename Unique<TypeList<typename _Pins::Port...>>::type;
            // Auxiliary class for operate on used ports
            using PortWriter = Private::PortsWriter<TypeList<_Pins...>, UsedPorts>;
        public :
            using PinsAsTypeList = TypeList<_Pins...>;
            // Data type fort pin list
            using DataType = typename SuitableUnsignedType<sizeof...(_Pins)>::type;
            const static unsigned int Length = sizeof...(_Pins);
            
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
            static void SetPullMode(DataType mask, PullMode pull);
            
            /**
             * @brief Set pull mode with mask (template method)
             * 
             * @tparam mask Mask
             * @tparam pull Pull mode
             * 
             * @par Returns
             * 	Nothing
             */
            template<DataType mask, PullMode pull>
            static void SetPullMode();
            
            /**
             * @brief Set pull mode
             * 
             * @tparam pull Pull mode
             * 
             * @par Returns
             * 	Nothing
             */
            template<PullMode pull>
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
            static void SetDriverType(DataType mask, DriverType driver);
            
            /**
             * @brief Set driver type with mask (template method)
             * 
             * @tparam mask Mask
             * @tparam driver Driver type
             * 
             * @par Returns
             * 	Nothing
             */
            template<DataType mask, DriverType driver>
            static void SetDriverType();
            
            /**
             * @brief Set driver type
             * 
             * @tparam driver Driver type
             * 
             * @par Returns
             * 	Nothing
             */
            template<DriverType driver>
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
            const static int IndexOf = TypeIndex<Pin, PinsAsTypeList>::value;
            
            /**
             * @brief Returns pin by index
             * 
             * @tparam Index Index
             */
            template<int Index>
            using Pin = typename GetType<Index, PinsAsTypeList>::type;
        };
    }
}

#include "impl/pinlist.h"

#endif //! ZHELE_PINLIST_COMMON_H