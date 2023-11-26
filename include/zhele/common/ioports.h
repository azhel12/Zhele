/**
 * @file
 * Base for GPIO ports
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_IOPORTS_COMMON_H
#define ZHELE_IOPORTS_COMMON_H

#include "ioreg.h"
#include "template_utils/type_list.h"

#include <zhele/clock.h>

#include <numeric>

namespace Zhele
{
    namespace IO
    {
        /**
         * @brief
         * Base class for GPIO port
         */
        class NativePortBase
        {
        public:
            using DataType = uint16_t;
            // Port configuartion
            enum Configuration
            {
                In = 0,
                Out = 1,
                AltFunc = 2,
                Analog = 3
            };

            // Pull mode
            enum PullMode
            {
                NoPull = 0,
                PullUp   = 1,
                PullDown = 2
            };

            // Port driver type
            enum DriverType
            {
                PushPull = 0,	///< Push-pull
                OpenDrain = 1	///< Open-drain
            };

            // Port speed
            enum Speed
            {
                Slow = 0, ///< Slow (< 2 MHz)
                Medium = 1, ///< Medium (< 10 MHz)
                Fast = 2, ///< Fast (< 50MHz)
                Fastest = 3 ///< Fastest (< 50MHz)
            };
            
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
        };

        class NullPort : public NativePortBase
        {
        public:
            using DataT = uint8_t;

            static void Write(DataT) {}
            template<DataT>
            static void Write() {}
            
            static DataT Read() { return DataT(); }

            static void Clear(DataT) {}
            template<DataT>
            static void Clear() {}

            static void Set(DataT) {}
            template<DataT>
            static void Set() {}

            static void ClearAndSet(DataT, DataT) {}
            template<DataT, DataT>
            static void ClearAndSet() {}

            static void Toggle(DataT) {}
            template<DataT>
            static void Toggle() {}

            static DataT PinRead() { return DataT(); }

            static void SetConfiguration(Configuration, DataT) {}
            template<Configuration, DataT>
            static void SetConfiguration() {}

            static void SetDriverType(DriverType, DataT) {}
            template<DriverType, DataT>
            static void SetDriverType() {}

            static void SetPullMode(PullMode, DataT) {}
            template<PullMode, DataT>
            static void SetPullMode() {}

            static void SetSpeed(Speed, DataT) {}
            template<Speed, DataT>
            static void SetSpeed() {}

            static void AltFuncNumber(uint8_t, DataT) {}
            template<uint8_t, DataT>
            static void AltFuncNumber() {}

            static void Enable() {}
            static void Disable() {}

            enum{Id = '-'};
        };

        namespace Private
        {
            /**
             * @brief Implement GPIO port
             */
            template<typename _Regs, typename _ClkEnReg, int ID>
            class PortImplementation : public NativePortBase
            {
            public:
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
                
                enum { Id = ID };

            private:
                static constexpr inline unsigned UnpackConfig2bits(unsigned mask, unsigned value, unsigned configuration);
                static constexpr inline unsigned UnpackConfig4Bit(unsigned mask, unsigned value, unsigned configuration);
            };
        }

        /**
         * @brief Implements IO ports list class.
         * 
         * @tparam _Ports IOPorts types.
         */
        template<typename... >
        class PortList;

        template<typename... _Ports>
        class PortList<TemplateUtils::TypeList<_Ports...> >
        {
        public:
            /**
             * @brief Enables all ports.
             * 
             * @par Returns
             *  Nothing
             */
            static void Enable()
            {
                (_Ports::Enable(), ...);
            }

            /**
             * @brief Disables all ports.
             * 
             * @par Returns
             *  Nothing
             */
            static void Disable()
            {
                (_Ports::Disable(), ...);
            }
        };

#define MAKE_PORT(REGS, ClkEnReg, className, ID) \
       namespace Private{\
            IO_STRUCT_WRAPPER(REGS, className ## Regs, GPIO_TypeDef);\
        }\
        using className = Private::PortImplementation<\
            Private::className ## Regs, \
            ClkEnReg,\
            ID>; \

        #if defined(GPIOA)
            MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
        #endif

        #if defined(GPIOB)
            MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
        #endif

        #if defined(GPIOC)
            MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')
        #endif

        #if defined(GPIOD)
            MAKE_PORT(GPIOD, Clock::PortdClock, Portd, 'D')
        #endif

        #if defined(GPIOE)
            MAKE_PORT(GPIOE, Clock::PorteClock, Porte, 'E')
        #endif

        #if defined(GPIOF)
            MAKE_PORT(GPIOF, Clock::PortfClock, Portf, 'F')
        #endif

        #if defined(GPIOG)
            MAKE_PORT(GPIOG, Clock::PortgClock, Portg, 'G')
        #endif
    }
}

#include "impl/ioports.h"

#endif //ZHELE_IOPORTS_COMMON_H
